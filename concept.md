The basic concept is to replace certain LaTeX with expressions that are faster to type, making it easier for situations
such as note-taking etc.

Syntax would be Pythonesque, with optionally named parameters

Basic replacements:

- `\integral{expr=f(x), wrt=x, from=a, to=b}` expands to `\int_{a}^{b} f(x)\, \mathrm{d}x`

- `\summ{expr=f(x), wrt=x, from=a, to=b}` expands to `\sum_{x=a}^{b} f(x)`

Both can take an "inf" argument which will output `\int_{\infty}^{\infty}` and `\sum_{\infty}^{\infty}`, respectively,
if true

Other features:

- Expand `(` and `)` in math mode to `\left(` `\right)`
