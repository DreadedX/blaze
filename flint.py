#!/usr/bin/env python3
import os
import errno
import configparser
import urllib.request
import shutil
import subprocess
from subprocess import PIPE
import stat
import sys
import argparse

# @todo Add update, gets latest version and stores the specific version
parser = argparse.ArgumentParser(add_help=False)
parser.add_argument("--config", action='store_const', const=True, default=False)
parser.add_argument("--version")
parser.add_argument("--debug-flint", action='store_const', const=True, default=False)
args, unknownargs = parser.parse_known_args()

flintdir = ".flint"
flintbin = "flint"
flintlock = ".flintlock"

if os.name == "nt":
    flintbin += ".exe"

# Make sure the flint directory exists
if not os.path.exists(flintdir):
    try:
        os.makedirs(flintdir)
    except OSError as exc:
        if exc != errno.EEXIST:
            raise

# Make sure the flintlock file exists
if not os.path.isfile(flintlock):
    # Create a new flintlock file with default values
    config = configparser.ConfigParser()
    with open(flintlock, "w") as f:
        config.write(f)

# Open the flintlock file
config = configparser.ConfigParser()
with open(flintlock, "r") as f:
    config.read_file(f)

# Make sure that all required keys exist
if not config.has_section("flint.py"):
    config["flint.py"] = {}
if not config.has_option("flint.py", "version"):
    config["flint.py"]["version"] = "nightly"

with open(flintlock, "w") as f:
    config.write(f)

# Check if the user has enable config mode
if args.config:
    # @todo Improve config mode
    if args.version:
        config["flint.py"]["version"] = args.version
        print("Flint will now use version: " + args.version)
    else:
        print("Did nothing...")

    with open(flintlock, "w") as f:
        config.write(f)
else:
    # Check desired version
    version = config["flint.py"]["version"]
    if args.version:
        version = args.version

    print("Flint: " + version)

    if os.path.isfile(version + "/" + flintbin):
        # The version contains a path to a flint executable
        flintdir = version
    else:
        # Check if we need to update
        needs_update = False
        if not os.path.isfile(flintdir + "/" + flintbin):
            needs_update = True
            print("Downloading flint...")
        else:
            # Download actual checksum
            checksum = configparser.ConfigParser()
            req = urllib.request.Request("https://downloads.mtgames.nl/release/flint/" + version + "/checksum")
            with urllib.request.urlopen(req) as response:
                checksum.read_string(response.read().decode("ascii"))

            if not os.path.isfile(flintdir + "/checksum"):
                needs_update = True

                with open(flintdir + "/checksum", "w") as f:
                    checksum.write(f)

                print("Downloading flint...")
            else:
                # Load the current checksum
                checksum_current = configparser.ConfigParser()
                with open(flintdir + "/checksum", "r") as f:
                    checksum_current.read_file(f)

                # Compare the checksum to determine if updates are needed
                # @note This does not detect if the executable is corrupted
                if not checksum_current["checksum"][flintbin] == checksum["checksum"]["flint"]:
                    needs_update = True

                    # Store the new checksum
                    checksum_current["checksum"][flintbin] = checksum["checksum"]["flint"]
                    with open(flintdir + "/checksum", "w") as f:
                        checksum_current.write(f)

                    print("Updating flint...")


        # @todo Show progress bar
        if needs_update:
            req = urllib.request.Request("https://downloads.mtgames.nl/release/flint/" + version + "/" + flintbin)
            with urllib.request.urlopen(req) as response, open(flintdir + "/" + flintbin, "wb") as f:
                shutil.copyfileobj(response, f)
            # Make sure we can execute flint
            mode = os.stat(flintdir + "/" + flintbin).st_mode
            os.chmod(flintdir + "/" + flintbin, mode | stat.S_IEXEC)

            with open(flintlock, "w") as f:
                config.write(f)

    command = []
    
    env = os.environ
    env["LD_LIBRARY_PATH"] = flintdir + ":.flint"

    seperator = "/"

    if os.name == "nt":
        command.extend([r"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat", "x64", "&&"])
        env["VSCMD_START_DIR"] = os.getcwd()
        seperator = "\\"

    if args.debug_flint:
        command.extend(["gdb", "--args"])
    command.append(flintdir + seperator + flintbin)
    command.extend(unknownargs)

    # Python pre 3.5 does not support run
    return_code = 0
    if sys.version_info[1] < 5:
        subprocess.call(command, env=env)
    else:
        return_code = subprocess.run(command, env=env).returncode

    exit(return_code)
