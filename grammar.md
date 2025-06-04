$$
\begin{align}
\langle\text{program}\rangle &\to \begin{cases} \langle\text{operator}\rangle~ \langle\text{program}\rangle \\
                              \epsilon \end{cases}
\\
\langle\text{operator}\rangle &\to operator~ (\langle\text{id\_list}\rangle~ op~ \langle\text{id\_list}\rangle)~ \langle\text{statements}\rangle
\\
\langle\text{id\_list}\rangle &\to \begin{cases} id~ \langle\text{id\_list}\rangle \\
                                     \epsilon \end{cases}
\\
\langle\text{statements}\rangle &\to \begin{cases} \langle\text{statement}\rangle~ \langle\text{statements}\rangle \\
                                            \epsilon \end{cases}
\\
\langle\text{statement}\rangle &\to \begin{cases} let~ id = \langle\text{expr}\rangle;\\
                       id = \langle\text{expr}\rangle;\\
                       \langle\text{access}\rangle = \langle\text{expr}\rangle; \\
                       return~ \langle\text{expr}\rangle; \\
                       \langle\text{expr}\rangle; \end{cases}
\\
\langle\text{expr}\rangle &\to \begin{cases} num \\
                    id \\
                    \langle\text{access}\rangle \\
                    (\langle\text{expr\_list}\rangle~ op~ \langle\text{expr\_list}\rangle) \\
                    if~ \langle\text{expr}\rangle~ then~ \langle\text{expr}\rangle~ else~ \langle\text{expr}\rangle
                    \end{cases} \\
\langle\text{expr\_list}\rangle &\to \begin{cases} \langle\text{expr}\rangle~ \langle\text{expr\_list}\rangle \\
                        \epsilon \end{cases}
\\
\langle\text{access}\rangle &\to [~ \langle\text{expr}\rangle~ ]
\end{align}
$$
