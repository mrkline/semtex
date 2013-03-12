# SemTeX - Streamlined LaTeX

## What is it?

SemTeX is a preprocessor for LaTeX which intends to make creating LaTeX documents simpler and faster.

### Syntax

- SemTeX generally accepts macros in the form of `\macroname{arg1, arg2}`. This is to keep SemTeX from looking out
  of place alongside normal LaTeX.
- Arguments that contain characters like `{`, `}`, and `,` can be enclosed in double quotes, i.e.
  `\macroname{"foo, bar", "biz{baz}"}`
- Arguments have optional names, e.g. `\macroname{arg1=one, arg2="two{foo}"}`
- Some macros have optional arguments. For example, the macro to create an integral `\integral`, has an optional
  `inf` argument that, when set to some form of true (`true`, `1`, `t`, etc.), fills in the bounds with infinity.  
  (i.e. `\integral{f(x), x, inf=t}` becomes `\integral_{-\infty}^{\infty} ...`)

- All unnamed arguments must go before named ones. Named arguments can be presented in any order.

### Planned Features

#### Basic replacements

- `\integral{expr=f(x), wrt=x, from=a, to=b}` expands to `\int_{a}^{b} f(x)\, \mathrm{d}x`

- `\summ{expr=f(x), wrt=x, from=a, to=b}` expands to `\sum_{x=a}^{b} f(x)`
  Both can take an "inf" argument which will output `\int_{\infty}^{\infty}` and `\sum_{\infty}^{\infty}`, respectively,
  if true

- `\->` expands to `\rightarrow`, `\=>` expands to `\Rightarrow`, `\<=>` expands to `\Leftrightarrow`, etc.

- `\unit{u=mV}` expands to `\,\mathrm{mV}`.

#### Graphviz integration

The following would expand into a Graphviz graph:

	\begin{dot}
	graph foo {
		foo -> bar;
		foo -> baz;
		baz -> biz;
	}
	\end{dot}

Options could be provided to determine whether the graph should be rendered with `dot` or with `dot2tex`, etc.

#### Miscellaneous features

- Expand `(` and `)` in math mode to `\left(` `\right)`

- Piecewise function definition:

	\begin{piecewise}{y(x) = }
	\piece{0, -\infty \leq x \leq 0}
	\piece{2x, x > 0}
	\end{piecewise}

- Automatically expand vertical space as needed in `array` tables (e.g. if a row contains `\frac`)

## Motivation

### Why?

I take notes in class using Vim and LaTeX and wanted to speed up the process.

### Why not [Vim-LaTeX](http://vim-latex.sourceforge.net/)? Why not make a LaTeX package?

Some things such as the simple search and replace macros (e.g. `\unit{V}` to `\,\mathrm{V}`) could easily be done either
as an addition to Vim-LaTeX or in a LaTeX package. However, some of the other planned features (such as automatic paren
resizing, etc.) would be rather difficult to do in either of these environments.

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
