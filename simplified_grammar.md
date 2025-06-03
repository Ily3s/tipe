$$
\begin{align}
[\text{program}] &\to [\text{operator}]^\text{*}
\\
[\text{operator}] &\to operator~(id^\text{*}~op~id^\text{*})~[\text{statement}]^\text{*}
\\
[\text{statement}] &\to \begin{cases} let~id = [\text{expr}];\\
                       id = [\text{expr}];\\
                       return~[\text{expr}]; \end{cases}
\\
[\text{expr}] &\to \begin{cases} num \\
                    ([\text{expr}]^\text{*}~op~[\text{expr}]^\text{*}) \\
                    if~[\text{expr}]~then~[\text{expr}]~else~[\text{expr}]
                    \end{cases}
\end{align}
$$
