A tool to colorize Pseudocode for Verbatim Latex.

> You need to add the lines:

```latex
\usepackage{fancyvrb}
\usepackage{xcolor}
```
> to the top of your document to have the code colored.

# Compile
In the repo:
```sh
make bin
```

# Usage
See:
```sh
./pseudotex -h
```
for Help

# Examples

```sh
./pseudotex input >> output.tex
```
To append input to an existing Tex file

```sh
./pseudotex -m "$(xclip -o -sel c)"| xclip -sel c
```
To directly convert your clipboard clipboard assuming you are using X
