"""Compile/run the production PDO helpers without ESPHome or physical hardware."""

import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", help="C++ compiler; defaults to c++, g++, clang++, or cl")
    parser.add_argument("--sanitize", action="store_true", help="Enable ASan/UBSan with GCC or Clang")
    args = parser.parse_args()
    compiler = args.compiler or next((path for name in ("c++", "g++", "clang++", "cl") if (path := shutil.which(name))), None)
    if compiler is None:
        parser.error("No C++ compiler found; on Windows run from a Visual Studio developer shell")
    build = ROOT / "tests/.esphome/decoder"
    build.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=build) as directory:
        output = Path(directory) / ("checks.exe" if Path(compiler).name.lower() in ("cl", "cl.exe") else "checks")
        include = ROOT / "components/zehnder_comfoair_q"
        source = ROOT / "tests/pdo_decode_checks.cpp"
        if Path(compiler).name.lower() in ("cl", "cl.exe"):
            if args.sanitize:
                parser.error("--sanitize requires GCC or Clang")
            command = [compiler, "/nologo", "/std:c++17", "/EHsc", "/W4", "/WX", f"/I{include}", str(source), f"/Fe:{output}"]
        else:
            command = [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{include}", str(source), "-o", str(output)]
            if args.sanitize:
                command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        subprocess.run(command, cwd=directory, check=True)
        subprocess.run([str(output)], check=True)


if __name__ == "__main__":
    main()
