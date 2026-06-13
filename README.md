# cmc - Copy Multiple Contents

cmc is a command-line utility that collects the contents of multiple files and
outputs them to the system clipboard, a file, or standard output. It is designed
for preparing source code and project context for LLMs.

## Usage

    cmc [OPTIONS] [PATHS...]

### Examples

    cmc main.c
    cmc src/*.c include/*.h
    cmc -R src -e *.test.c -o output.txt
    cmc -R . -p -c

### Options

| Short | Long                  | Description                         |
|-------|-----------------------|-------------------------------------|
| `-R`  | `--recursive`         | Recursively scan directories        |
| `-e`  | `--exclude PATTERN`   | Exclude files matching a glob       |
| `-E`  | `--exclude-file FILE` | Load exclusion patterns from file   |
| `-o`  | `--output FILE`       | Write output to FILE                |
| `-c`  | `--clipboard`         | Copy output to system clipboard     |
| `-s`  | `--symlinks`          | Follow symbolic links               |
| `-p`  | `--paths`             | Prepend each file with its path     |
| `-b`  | `--binary`            | Include binary files                |
| `-h`  | `--help`              | Display help                        |
| `-v`  | `--version`           | Display version information         |

## Building

    make

Run tests:

    cd tests && ./test_basic.sh

## Installing

    sudo make install

## Packaging

    dpkg-buildpackage -b -uc

## Contributing

Contributions are welcome.

You can:

- Propose new features
- Report bugs
- Improve performance or CLI behavior
- Submit fixes or refactoring

## License

This project is licensed under the GNU GPLv3. See 'LICENSE' for details.
