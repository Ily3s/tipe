$$
\begin{align}
[\text{program}] &\to \begin{cases} [\text{operator}] [\text{program}] \\
                              \epsilon \end{cases}
\\
[\text{operator}] &\to operator\:([\text{id\_list}]\:op\:[\text{id\_list}])\:[\text{statements}]
\\
[\text{id\_list}] &\to \begin{cases} id\:[\text{id\_list}] \\
                                     \epsilon \end{cases}
\\
[\text{statements}] &\to \begin{cases} [\text{statement}]\: [\text{statements}] \\
                                            \epsilon \end{cases}
\\
[\text{statement}] &\to \begin{cases} let\:id = [\text{expr}];\\
                       id = [\text{expr}];\\
                       return [\text{expr}]; \end{cases}
\\
[\text{expr}] &\to \begin{cases} num \\
                    ([\text{expr\_list}]\:op\:[\text{expr\_list}]) \\
                    if\:[\text{expr}]\:then\:[\text{expr}]\:else\:[\text{expr}]
                    \end{cases} \\
[\text{expr\_list}] &\to \begin{cases} [\text{expr}]\:[\text{expr\_list}] \\
                        \epsilon \end{cases}
\end{align}
$$
