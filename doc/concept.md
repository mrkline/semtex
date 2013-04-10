The basic concept is to replace certain LaTeX with expressions that are faster to type, making it easier for situations
such as note-taking etc.

Syntax would be Pythonesque, with optionally named parameters

Basic replacements:

- In math mode, `\integral{expr=f(x), wrt=x, from=a, to=b}` expands to `\int_{a}^{b} f(x)\, \mathrm{d}x`

- In math mode, `\summ{expr=f(x), wrt=x, from=a, to=b}` expands to `\sum_{x=a}^{b} f(x)`
  Both can take an "inf" argument which will output `\int_{\infty}^{\infty}` and `\sum_{\infty}^{\infty}`, respectively,
  if true

- In math and normal mode,`\->` expands to `\rightarrow`, `\=>` expands to `\Rightarrow`,
  `\<=>` expands to `\Leftrightarrow`, etc.

- In math mode, `\unit{u=mV} expands to `\, \mathrm{mV}`.

Graphviz Integration:

- The following would expand into a Graphviz graph:

	\begin{dot}
	graph foo {
		foo -> bar;
		foo -> baz;
		baz -> biz;
	}
	\end{dot}

  Options could be provided to determine whether the graph should be rendered with `dot` or with `dot2tex`, etc.

Miscellaneous features:

- Expand `(` and `)` in math mode to `\left(` `\right)`

- Piecewise function definition:

	\begin{piecewise}{y(x) = }
	\piece 0, & -\infty \leq x \leq 0
	\piece 2x, & x > 0
	\end{piecewise}

- Automatically expand vertical space as needed in `array` tables (e.g. if a row contains `\frac`)
