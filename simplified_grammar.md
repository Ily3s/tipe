$$
\begin{align}
[\text{program}] &\to \begin{cases} [\text{operator}] [\text{program}] \\
                              \epsilon \end{cases}
\\
[\text{operator}] &\to operator\:(id^*\:op\:[\text{id\_list}])\:[\text{statement}]^*
\\
[\text{statement}] &\to \begin{cases} let\:id = [\text{expr}];\\
                       id = [\text{expr}];\\
                       return [\text{expr}]; \end{cases}
\\
[\text{expr}] &\to \begin{cases} num \\
                    ([\text{expr}]^*\:op\:[\text{expr}]^*) \\
                    if\:[\text{expr}]\:then\:[\text{expr}]\:else\:[\text{expr}]
                    \end{cases}
\end{align}
$$
