#!/usr/bin/env python3
import os
import errno
import configparser
import urllib.request
import hashlib
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
    config["flint.py"] = {
            "version": "nightly"
    }
    with open(flintlock, "w") as f:
        config.write(f)

# Open the flintlock file
config = configparser.ConfigParser()
with open(flintlock, "r") as f:
    config.read_file(f)

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
            checksum = configparser.ConfigParser()
            req = urllib.request.Request("https://downloads.mtgames.nl/release/flint/" + version + "/checksum")
            with urllib.request.urlopen(req) as response:
                checksum.read_string(response.read().decode("ascii"))

            h = hashlib.sha256()
            with open(flintdir + "/" + flintbin, "rb") as f:
                buf = f.read(65536)
                while len(buf) > 0:
                    h.update(buf)
                    buf = f.read(65536)

            if not checksum["checksum"]["flint"] == h.hexdigest():
                needs_update = True
                print("Updating flint...")

        # @todo Show progress bar
        if needs_update:
            req = urllib.request.Request("https://downloads.mtgames.nl/release/flint/" + version + "/flint")
            with urllib.request.urlopen(req) as response, open(flintdir + "/" + flintbin, "wb") as f:
                shutil.copyfileobj(response, f)
            # Make sure we can execute flint
            mode = os.stat(flintdir + "/" + flintbin).st_mode
            os.chmod(flintdir + "/" + flintbin, mode | stat.S_IEXEC)

    command = []
    if args.debug_flint:
        command.extend(["gdb", "--args"])
    command.append(flintdir + "/" + flintbin)
    command.extend(unknownargs)

    env = os.environ
    env["LD_LIBRARY_PATH"] = flintdir

    return_code = subprocess.run(command, env=env).returncode
    exit(return_code)
