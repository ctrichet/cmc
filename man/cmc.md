# cmc

> Collect file contents and output to stdout, a file, or the system clipboard.
> More information: <https://github.com/ctrichet/cmc>.

- Print a single file to stdout:

`cmc {{file}}`

- Recursively scan a directory with path labels:

`cmc -R {{src/}} -p`

- Scan a directory excluding build artifacts:

`cmc -R {{.}} -e '{{*.o}}' '{{build/*}}'`

- Copy all files to the clipboard:

`cmc -R {{.}} -c`

- Write recursive scan to a file with exclusion config:

`cmc -R {{.}} -E '{{*.md}}' -o {{context.txt}}`

- Send source code to an LLM CLI tool:

`cmc -R {{src/}} -p | llm -s "{{Explain the architecture}}"`

- Include binary files:

`cmc -b {{image.png}} {{data.db}}`

- Display help:

`cmc --help`
