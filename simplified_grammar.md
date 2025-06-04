$$
\begin{align}
\langle\text{program}\rangle &\to \langle\text{operator}\rangle^\text{\*}
\\
\langle\text{operator}\rangle &\to operator~ (id^\text{\*}~ op~ id^\text{\*})~ \langle\text{statement}\rangle^\text{\*}
\\
\langle\text{statement}\rangle &\to \begin{cases} let~ id = \langle\text{expr}\rangle;\\
                       id = \langle\text{expr}\rangle;\\
                       \langle\text{access}\rangle = \langle\text{expr}\rangle;\\
                       return~ \langle\text{expr}\rangle; \\
                       \langle\text{expr}\rangle; \end{cases}
\\
\langle\text{expr}\rangle &\to \begin{cases} num \\
                    id \\
                    \langle\text{access}\rangle \\
                    (\langle\text{expr}\rangle^\text{\*}~ op~ \langle\text{expr}\rangle^\text{\*}) \\
                    if~ \langle\text{expr}\rangle~ then~ \langle\text{expr}\rangle~ else~ \langle\text{expr}\rangle
                    \end{cases}
\\
\langle\text{access}\rangle &\to [~ \langle\text{expr}\rangle~ ]
\end{align}
$$
