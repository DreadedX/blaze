#!/usr/bin/env python3
import os
import platform
import errno
import configparser
import urllib.request
import shutil
import subprocess
from subprocess import PIPE
import stat
import sys
import argparse

flintlock = ".flintlock"

def install_plugin(url, plugins_folder, config, local = False):
    metainfo = configparser.ConfigParser()

    if not local:
        req = urllib.request.Request(url + "/metainfo")
        with urllib.request.urlopen(req) as response:
            metainfo.read_string(response.read().decode("ascii"))
    else:
        with open(os.path.join(url, "metainfo.local"), "r") as f:
            metainfo.read_file(f)

    plugin_name = metainfo["meta"]["name"] + "@" + metainfo["meta"]["author"]

    # Make sure we can't have local and remote version of the same plugin at the same time
    # @todo This could be cleaner
    if config.has_option("flint.py-plugins", plugin_name if local else plugin_name + ".local"):
        config.remove_option("flint.py-plugins", plugin_name if local else plugin_name + ".local")

    print("Installing '" + plugin_name + "'")

    plugin_folder = os.path.join(plugins_folder, plugin_name)

    if os.path.exists(plugin_folder):
        shutil.rmtree(plugin_folder)
    os.makedirs(plugin_folder)

    # Register the plugin so that we can later update
    config["flint.py-plugins"][plugin_name if not local else plugin_name + ".local"] = url

    with open(flintlock, "w") as f:
        config.write(f)

    # @todo Use this same system for flint self
    # @todo Add multiplatform support
    if platform.system() == "Linux" and metainfo.has_section("linux"):
        for filename in metainfo["linux"]:
            if not local:
                req = urllib.request.Request(url + "/" + filename)
                with urllib.request.urlopen(req) as response, open(os.path.join(plugin_folder, filename), "wb") as f:
                    shutil.copyfileobj(response, f)
            else:
                os.symlink(os.path.abspath(os.path.join(url, metainfo["linux"][filename])), os.path.join(plugin_folder, filename))
    elif platform.system() == "Windows" and metainfo.has_section("windows"):
        for filename in metainfo["windows"]:
            if not local:
                req = urllib.request.Request(url + "/" + filename)
                with urllib.request.urlopen(req) as response, open(os.path.join(plugin_folder, filename), "wb") as f:
                    shutil.copyfileobj(response, f)
            else:
                # @todo This requires admin mode on windows
                os.symlink(os.path.abspath(os.path.join(url, metainfo["windows"][filename])), os.path.join(plugin_folder, filename))

    else:
        print("Plugin not available for current platform")

def remove_plugin(plugin_name, plugins_folder, config):
    plugin_folder = os.path.join(plugins_folder, plugin_name)

    if config.has_option("flint.py-plugins", plugin_name):
        config.remove_option("flint.py-plugins", plugin_name)
    elif config.has_option("flint.py-plugins", plugin_name + ".local"):
        config.remove_option("flint.py-plugins", plugin_name + ".local")
    else:
        print("Plugin '" + plugin_name + "' does not exist")
        return

    if os.path.exists(plugin_folder):
        shutil.rmtree(plugin_folder)

def main():
    # @todo Add update, gets latest version and stores the specific version
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--config", action='store_const', const=True, default=False)
    parser.add_argument("--local", action='store_const', const=True, default=False)
    parser.add_argument("--generate")
    parser.add_argument("--version")
    parser.add_argument("--install", nargs='?', const="__update__")
    parser.add_argument("--remove")
    parser.add_argument("--debug-flint", action='store_const', const=True, default=False)
    args, unknownargs = parser.parse_known_args()

    flintdir_base = ".flint"
    flintdir = os.path.join(flintdir_base, "versions")
    flintplug = os.path.join(flintdir_base, "plugins")
    flintbin = "flint"

    if os.name == "nt":
        flintbin += ".exe"

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
    if not config.has_option("flint.py", "base_url"):
        config["flint.py"]["base_url"] = "https://downloads.mtgames.nl/release/flint"
    if not config.has_section("flint.py-plugins"):
        config["flint.py-plugins"] = {}

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
    elif args.generate:
        if not os.path.exists(args.generate + "/src"):
            os.makedirs(args.generate + "/src")
        if not os.path.exists(args.generate + "/include"):
            os.makedirs(args.generate + "/include")

        # @todo Also generate a simlpe example cpp file
        with open("./flint.lua", "w") as f:
            f.write("executable \"" + args.generate + "\"\n")
            f.write("\tpath \"" + args.generate + "\"\n")
            f.write("\n")
            f.write("run_target \"" + args.generate + "\"\n")
    elif args.install:
        # @todo We need to auto update all plugins
        # @todo We need to be able to remove plugins
        metainfo = configparser.ConfigParser()
        if args.install == "__update__":
            for plugin in config["flint.py-plugins"]:
                # @todo Make the download functions actual functions so we can call them here
                install_plugin(config["flint.py-plugins"][plugin], flintplug, config, plugin.endswith(".local"))
        else:
            install_plugin(args.install, flintplug, config, args.local)
    
    elif args.remove:
        remove_plugin(args.remove, flintplug, config)

        with open(flintlock, "w") as f:
            config.write(f)
    else:
        # Check desired version
        version = config["flint.py"]["version"]
        base_url = config["flint.py"]["base_url"]
        if args.version:
            version = args.version

        print("Flint: " + version)

        flintdir = os.path.join(flintdir, version)

        if os.path.isfile(os.path.join(version, flintbin)):
            # The version contains a path to a flint executable
            flintdir = version
        else:
            # Make sure the flint directory exists
            if not os.path.exists(flintdir):
                try:
                    os.makedirs(flintdir)
                except OSError as exc:
                    if exc != errno.EEXIST:
                        raise

            # Check if we need to update
            needs_update = False

            # Download actual checksum
            checksums = configparser.ConfigParser()
            try:
                req = urllib.request.Request(base_url + "/" + version + "/checksums")
                with urllib.request.urlopen(req) as response:
                    checksums.read_string(response.read().decode("ascii"))

                checksums_current = configparser.ConfigParser()
                if (not os.path.isfile(os.path.join(flintdir, flintbin))) or (not os.path.isfile(os.path.join(flintdir, "checksums"))):
                    # We need to download instead of update
                    needs_update = True

                    checksums_current["flint"] = {}

                    print("Downloading flint...")
                else:
                    # Load the current checksum
                    with open(os.path.join(flintdir, "checksums"), "r") as f:
                        checksums_current.read_file(f)

                    # Compare the checksum to determine if updates are needed
                    # @note This does not detect if the executable is corrupted
                    if not checksums_current["flint"][flintbin] == checksums["flint"][flintbin]:
                        needs_update = True
                        print("Updating flint...")

                # Store the new checksum
                checksums_current["flint"][flintbin] = checksums["flint"][flintbin]
                with open(os.path.join(flintdir, "checksums"), "w") as f:
                    checksums_current.write(f)

            except urllib.error.URLError as error:
                print("Unable to update flint!")
                needs_update = False

            # @todo Show progress bar
            if needs_update:
                try:
                    req = urllib.request.Request(base_url + "/" + version + "/" + flintbin)
                    with urllib.request.urlopen(req) as response, open(flintdir + "/" + flintbin, "wb") as f:
                        shutil.copyfileobj(response, f)
                    # Make sure we can execute flint
                    mode = os.stat(os.path.join(flintdir, flintbin)).st_mode
                    os.chmod(os.path.join(flintdir, flintbin), mode | stat.S_IEXEC)

                    with open(flintlock, "w") as f:
                        config.write(f)
                except urllib.error.URLError as error:
                    print("Flint update failed!")
                    needs_update = False

        command = []
        
        env = os.environ
        if os.name == "nt":
            env["PATH"] = flintdir + ";.flint/plugins;" + env["PATH"]
        else:
            env["LD_LIBRARY_PATH"] = flintdir + ":.flint/plugins"

        if os.name == "nt":
            command.extend([r"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat", "x64", "&&"])
            env["VSCMD_START_DIR"] = os.getcwd()

        if args.debug_flint:
            if platform.system() == "Linux":
                command.extend(["gdb", "--args"])
            elif platform.system() == "Windows":
                command.extend(["devenv", "/debugexe"])
            else:
                print("Debugging not available for current platform")

        command.append(os.path.join(flintdir, flintbin))
        command.extend(unknownargs)

        # Python pre 3.5 does not support run
        return_code = 0
        if sys.version_info[1] < 5:
            subprocess.call(command, env=env)
        else:
            return_code = subprocess.run(command, env=env).returncode

        exit(return_code)

if __name__ == "__main__":
    main()
