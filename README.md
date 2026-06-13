# cmc — Copy Multiple Contents

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-GPLv3-green)
![Platform](https://img.shields.io/badge/platform-linux-lightgrey)
![C standard](https://img.shields.io/badge/C-17-555555)

**cmc** collects the contents of multiple files and outputs them to **stdout**, a **file**, or the **system clipboard** — in one command. It is purpose-built for preparing source code and project context for Large Language Models (LLMs).

Instead of manually opening five files, selecting text, copying, switching tabs, pasting, and repeating — `cmc` does it all in a single command.

```bash
# Before: 15 seconds of tedious clicking
# After:
cmc -R src/ -p -c   # whole project → clipboard in 0.1s
```

---

## Table of Contents

- [Installation](#installation)
- [Quick start](#quick-start)
- [Real-world workflows](#real-world-workflows)
- [Full CLI reference](#full-cli-reference)
- [Selection vs exclusion](#selection-vs-exclusion)
- [Binary files](#binary-files)
- [Clipboard backends](#clipboard-backends)
- [Output format](#output-format)
- [Output destinations](#output-destinations)
- [Exit codes](#exit-codes)
- [Building](#building)
- [Testing](#testing)
- [Packaging](#packaging)
- [Editor integration](#editor-integration)
- [FAQ / Troubleshooting](#faq--troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## Installation

### Prerequisites

- `gcc`, `make`
- `libmagic-dev` (for MIME-type binary detection)

### From source

```bash
git clone https://github.com/<USER>/cmc.git
cd cmc
make
sudo make install
```

If `libmagic-dev` headers/libraries are not installed system-wide:

```bash
apt-get download libmagic-dev
dpkg-deb -x libmagic-dev_*.deb /tmp/libmagic-dev
make MAGIC_DIR=/tmp/libmagic-dev/usr
sudo make install
```

### From Debian package

```bash
sudo dpkg -i cmc_1.0-1_amd64.deb
sudo apt install -f
```

### Verify

```bash
cmc --version
```

---

## Quick start

### Single file → stdout

```bash
cmc main.c
```

Prints the contents of `main.c` to the terminal.

### Multiple files

```bash
cmc main.c helper.c config.h
```

Concatenates the three files in lexicographic order.

### Entire project → clipboard

```bash
cmc -R src/ -p -c
```

Recursively collects all files under `src/`, prepends each with its relative path header, and copies everything to the clipboard.

### Directory with exclusions

```bash
cmc -R . -e "*.o" "build/*" "__pycache__/*"
```

Once `-e` is encountered, all subsequent positional arguments become exclusion patterns. Scans the current directory recursively, skipping object files, the build directory, and Python cache.

### Specific file types only

```bash
cmc -R . -e ".git/*" "node_modules/*" -p -o ctx.txt
```

All files under `.`, excluding `.git` and `node_modules`, with path labels, written to `ctx.txt`.

---

## Real-world workflows

### Feed context directly to an LLM CLI tool

```bash
# With Simon Willison's 'llm' CLI
cmc -R src/ -p | llm -s "Explain the architecture of this project"

# With 'sgpt' (ShellGPT)
cmc src/*.py -p | sgpt "Find bugs in these files"

# With a plain curl prompt to an API
cmc -R . -e "*.md" -p | jq -Rs '{contents: ., question: "Review this code"}' | \
  curl -s https://api.openai.com/v1/chat/completions -H "Authorization: Bearer $OPENAI_KEY" -d @-
```

### Send specific files to clipboard for a PR description

```bash
cmc src/CHANGELOG.md src/MIGRATION.md -c
```

### Collect logs for debugging

```bash
cmc -R /var/log/myapp/ -e "*.gz" -p -o debug_context.txt
```

### Combine with grep for targeted context

```bash
# Find files mentioning "auth" then feed them to cmc
grep -rl "auth" --include="*.rs" src/ | xargs cmc -p -c
```

### Save project snapshot for review

```bash
cmc -R . -E "*.md" -p -o snapshot.md
```

---

## Full CLI reference

```
cmc [OPTIONS] [PATHS...]
```

| Short | Long                  | Description                         |
|-------|-----------------------|-------------------------------------|
| `-R`  | `--recursive`         | Recursively scan directories        |
| `-e`  | `--exclude PATTERN`   | Exclude files matching a POSIX glob |
| `-E`  | `--excludes PATTERN`  | Exclude files matching a glob (also loads $XDG_CONFIG_HOME/cmc/.cmc_excludes) |
| `-o`  | `--output FILE`       | Write output to FILE                |
| `-c`  | `--clipboard`         | Copy output to system clipboard     |
| `-s`  | `--symlinks`          | Follow symbolic links               |
| `-p`  | `--paths`             | Prepend each file with its path     |
| `-b`  | `--binary`            | Include binary files                |
| `-h`  | `--help`              | Display help and exit               |
| `-v`  | `--version`           | Show version and exit               |

### Constraints

- `-c` and `-o` cannot be used together (exit code 2).
- `-s` enables following symbolic links in all scan modes (without it, symlinks are skipped).
- `-b` is rarely needed — `libmagic` detection is reliable for most binary formats.

---

## Selection vs exclusion

Arguments are parsed **left to right**.

- **Before** the first `-e` / `-E`: all positional arguments are **selection patterns** (POSIX globs or literal paths). Only files matching at least one pattern are included.
- **After** `-e` / `-E`: all positional arguments are **exclusion patterns** (POSIX globs). Matching files are skipped.
- If **no selection pattern** is given, all discovered files are included (subject to exclusions).

```
     selection patterns     exclusion patterns
     ┌───────────────┐  ┌──────────────────────────┐
  cmc *.c *.h         -e *_test.* *_mock.*
```

```bash
# Select .c and .h files, but exclude test/mock files
cmc *.c *.h -e *_test.* *_mock.*

# Select everything under src/, exclude what's in $XDG_CONFIG_HOME/cmc/.cmc_excludes
cmc -R src/ -E "*.o"

# Only exclude, no selection: grab everything except .o files
cmc -R . -e "*.o"

# Multiple exclusion patterns with a single -e
cmc -R src/ -e "*_test.go" "*_mock.go" "*.pb.go"
```

### Inclusion logic

A file is copied **only if** all three conditions are met:

```
(matches a selection pattern  OR  no selection patterns exist)
AND
(does not match any exclusion pattern)
AND
(not a binary file  OR  --binary is set)
```

---

## Exclusion config file

When `-E` / `--excludes` is used, cmc automatically loads exclusion patterns from a fixed user-config location:

| If `$XDG_CONFIG_HOME` is set | `$XDG_CONFIG_HOME/cmc/.cmc_excludes` |
|------------------------------|--------------------------------------|
| Default                      | `~/.config/cmc/.cmc_excludes`        |

The file format is one glob pattern per line, with `#` for comments (same as the patterns passed via `-e` / `-E` on the command line).

Example `.cmc_excludes`:

```
# Build artifacts
build/*
target/*
dist/*

# Dependencies
node_modules/*
vendor/*

# Cache / metadata
.git/*
__pycache__/*
*.pyc
.eggs/*

# IDE files
*.swp
*.swo
.vscode/*
.idea/*

# Media (usually binary)
*.png
*.jpg
*.pdf
*.zip
```

Usage — patterns from command line AND from file are combined:

```bash
cmc -R . -E "*.txt" -p -c
```

If `.cmc_excludes` does not exist, a warning is printed to stderr and cmc continues with only the command-line patterns. A sample file is provided as `.cmc_excludes.example` in the project repository.

This replaces the old `--exclude-file` behavior (which required an explicit file path argument).

---

## Binary files

By default, binary files are **silently excluded**. Detection uses `libmagic` to inspect MIME types (ELF executables, images, PDFs, archives, etc.).

To include binaries:

```bash
cmc -b image.png data.db
```

Use with caution — binary content will produce garbled text in your terminal or clipboard.

---

## Clipboard backends

| Environment | Tool              | Install                          |
|-------------|-------------------|----------------------------------|
| Wayland     | `wl-copy`         | `sudo apt install wl-clipboard`  |
| X11         | `xclip`           | `sudo apt install xclip`         |

Detection is automatic. If neither tool is found, `cmc -c` exits with code 4.

---

## Output format

Without `-p`, cmc outputs raw file contents concatenated in order.

### With `-p` (path labels)

```
### FILE: src/main.c ###
int main(void) {
    return 0;
}

### FILE: src/util.c ###
int add(int a, int b) {
    return a + b;
}
```

The `### FILE: <path> ###` header makes it easy for LLMs to associate code with filenames, and for users to navigate long concatenated outputs.

### Output order

Files are emitted in **lexicographical ascending order** of their relative paths.

```
a/file.txt
b/file.txt
c/file.txt
```

---

## Output destinations

| Flags       | Destination        |
|-------------|--------------------|
| *(none)*    | stdout             |
| `-o FILE`   | FILE (overwrites)  |
| `-c`        | System clipboard   |

`-c` and `-o` are **mutually exclusive** — using both exits with code 2.

---

## Exit codes

| Code | Meaning                        |
|------|--------------------------------|
| 0    | Success                        |
| 1    | General error (I/O, memory)    |
| 2    | Invalid command line arguments |
| 3    | Output file error              |
| 4    | Clipboard tool not found       |
| 5    | libmagic initialization error  |

Scripts can check `$?` to distinguish failure modes:

```bash
cmc -R . -c || case $? in
  4) echo "Install wl-clipboard or xclip" ;;
  2) echo "Bad arguments" ;;
esac
```

---

## Building

```bash
make
```

Build with a custom libmagic path:

```bash
make MAGIC_DIR=/path/to/libmagic-dev/usr
```

Clean build artifacts:

```bash
make clean
```

---

## Testing

```bash
make check
```

Or run individual test suites:

```bash
./tests/test_basic.sh
./tests/test_recursive.sh
./tests/test_exclude.sh
./tests/test_exclude_file.sh
./tests/test_exclusion_selection.sh
./tests/test_error.sh
./tests/test_paths.sh
./tests/test_file_output.sh
./tests/test_mutex.sh
./tests/test_separator.sh
./tests/test_symlink.sh
./tests/test_symlink_flat.sh
./tests/test_trailing_slash.sh
./tests/test_binary.sh
./tests/test_clipboard.sh
```

Tests are shell scripts that exercise cmc end-to-end and validate exit codes, output content, and edge cases (empty directories, missing files, recursive symlinks, binary detection, clipboard integration, exclusion config).

---

## Packaging

```bash
sudo apt install devscripts libmagic-dev
dpkg-buildpackage -b -us -uc
```

The `.deb` is generated in the parent directory.

---

## Editor integration

### Neovim

Map a key to dump the current project into a new buffer:

```lua
vim.keymap.set("n", "<leader>cc", function()
  local buf = vim.api.nvim_create_buf(false, true)
  vim.api.nvim_set_current_buf(buf)
  vim.fn.systemlist("cmc -R . -p", {}, "silent")
end, { desc = "cmc: load project context into buffer" })
```

### VS Code

Add a task in `.vscode/tasks.json`:

```json
{
  "version": "2.0.0",
  "tasks": [{
    "label": "cmc: copy project to clipboard",
    "type": "shell",
    "command": "cmc -R . -p -c",
    "problemMatcher": []
  }]
}
```

### Shell alias

```bash
alias ctx='cmc -R . -p -c'
alias ctxo='cmc -R . -p -o context.txt'
```

---

## FAQ / Troubleshooting

### "cmc: error: clipboard: wl-copy not found"

Install the appropriate clipboard tool:

```bash
# Wayland
sudo apt install wl-clipboard

# X11
sudo apt install xclip
```

### "Binary files are being skipped and I want them included"

Pass `-b` / `--binary`. Note that binary content will produce unreadable output.

### "How do I use cmc in a pipe?"

cmc writes to stdout by default, so it composes naturally:

```bash
cmc src/*.py -p | wc -l
cmc -R . -p | head -100
cmc -R . -p | grep -n "TODO"
```

### "Why not just use cat / xclip / find / etc.?"

You can, but it requires a chain of tools with careful handling of delimiters, recursion, and binary detection. `cmc` packages all of that into a single predictable command with a consistent output format designed for LLM consumption.

### "Does cmc follow symlinks?"

Only with `-s` / `--symlinks`. Without it, symlinks are skipped.

### "What encoding does cmc assume?"

cmc treats all files as raw bytes. It does not perform encoding conversion — files are copied verbatim.

### "Is cmc safe for secrets?"

cmc copies whatever file contents it is asked to copy. **Do not** run it on directories that contain secrets, `.env` files, API keys, or private keys unless you intend to expose them. A `.cmc_excludes` file can help exclude sensitive files.

### "Can cmc handle very large files or projects?"

cmc buffers the entire output in memory before writing. Very large files (hundreds of MB) will consume proportional memory. For typical source code projects this is not an issue.

### "The output seems empty — what went wrong?"

Check that:
- Your selection patterns actually match files
- Your exclusion patterns aren't accidentally excluding everything
- Files are not all binary (use `-b` to test)
- The paths exist and are readable

### "Does cmc support non-Linux OS?"

Currently **Linux only**. The code uses POSIX APIs (`nftw`, `getopt_long`, `fork`/`exec`) that would need adaptation for macOS/BSD (different `nftw` signature) and Windows (no POSIX subsystem).

---

## Contributing

Contributions are welcome!

- [Open an issue](https://github.com/<USER>/cmc/issues) for bugs or feature requests
- Submit a pull request with a clear description
- Ensure all tests pass: `make check`
- Follow existing code style:
  - C17, POSIX APIs
  - No trailing whitespace
  - `gcc -Wall -Wextra -Werror` clean
- If adding a feature, include a test

---

## License

Copyright (C) 2024-2025  cmc contributors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

See [LICENSE](LICENSE) for details.

GPLv3 was chosen over AGPLv3 (which adds a network-server source requirement unnecessary for a CLI tool) and MIT (which would allow proprietary redistribution without source). GPLv3 ensures that all modifications remain free and open source, matching the project's goal of serving the open-source LLM tooling ecosystem.
