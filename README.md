# SemTeX - Streamlined LaTeX

## What is it?

SemTeX is a preprocessor for LaTeX aimed at simplifying LaTeX syntax and making documents easier to type.

### Syntax

- SemTeX generally accepts macros in the form of `\macroname{arg1}{arg2}`. This is to keep SemTeX from looking out
  of place alongside normal LaTeX.
- Arguments can be nested (in most cases - some do not allow nesting).
- Some macros have options that can be set. For example, the macro to create an integral `\integral`, has an `inf`
  option that fills in the bounds with infinity.
  (i.e. `\integral[inf]{f(x)}{x}` becomes `\int_{-\infty}^{\infty} ...`)

### Implemented Features

#### Basic Replacements

- `-->` expands to `\rightarrow`, `==>` expands to `\Rightarrow`, `<==>` expands to `\Leftrightarrow`, etc.

- `!=` expands to `\neq`, `>=` expands to `\geq`, `<=` expands to `\leq`, etc.

- `"w` expands to `\omega`, `"p` expands to `\pi`, `"F` expands to `\Phi`, etc.

#### Macros

- `\integral{f(x)}{x}{a}{b}` expands to `\int_{a}^{b} f(x)\, \mathrm{d}x`

- `\summ{x}{a}{b}` expands to `\sum_{x=a}^{b}`
  Both can take an "inf" option which will output `\int_{\infty}^{\infty}` and `\sum_{\infty}^{\infty}`, respectively,
  if set. They can also take a "mir" option, which mirrors a single argument on both sides
  (e.g. `\summ[mir]{x}{a}` becomes `\sum_{x = -a}^{a}`

- `\deriv{y}{x}{2}` expands to `\frac{\mathrm{d}^{2}y}{\mathrm{d}x^{2}}`,
  `\deriv{y}{x}` expands to `\frac{\mathrm{d}y}{\mathrm{d}x}`,
  and `\deriv{x}` expands to `\frac{\mathrm{d}}{\mathrm{d}x}`.

- `\unit{mV}` expands to `\,\mathrm{mV}`.

- Piecewise function definition:

```latex
\begin{piecewise}{y(x)}
\piece{0}{-\infty <= x <= 0}
\piece{2x}{x > 0}
\end{piecewise}
```

### Planned Features

#### Graphviz integration

The following would expand into a Graphviz graph:

```latex
\begin{dot}
graph foo {
	foo -> bar;
	foo -> baz;
	baz -> biz;
}
\end{dot}
```

Options could be provided to determine whether the graph should be rendered with `dot` or with `dot2tex`, etc.

#### Miscellaneous features

- Expand `(` and `)` in math mode to `\left(` `\right)`

- Automatically expand vertical space as needed in `array` tables (e.g. if a row contains `\frac`)

## Dependencies

To run SemTeX:
- LaTeX (specifically pdflatex)
- LaTeX math packages (usually provided by the `texlive-extra` package)

To build SemTeX:
- A compiler that supports (a good chunk of) C++11
- [Boost](http://www.boost.org/) (used for `boost::regex` and `boost::filesystem`).
  I planned on using `std::regex` but gcc has yet to provide a working implementation.
- [TCLAP](http://tclap.sourceforge.net/) (a small C++ command line argument parser)

## Motivation

### Why?

I take notes using Vim and LaTeX and wanted to speed up the process.

### Why not [Vim-LaTeX](http://vim-latex.sourceforge.net/)? Why not make a LaTeX package?

Some things such as the simple search and replace macros (e.g. `\unit{V}` to `\,\mathrm{V}`) could easily be done either
as an addition to Vim-LaTeX or in a LaTeX package. However, some of the other planned features (such as automatic paren
resizing, macros with options etc.) would be rather difficult to do in either of these environments.

Additionally, it is the hope that some SemTeX macros and replacements are easier to read through than their LaTeX
equivalents (for example, `here <-- there` versus `here \leftarrow there`).

## License (zlib license)

Copyright (C) 2013 Matt Kline

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
