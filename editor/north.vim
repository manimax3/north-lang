syn region northFold start=/do/ end=/end/ contains=ALL transparent keepend fold

syn keyword Function proc
syn keyword Keyword variable do end return
syn keyword Operator dup drop swap rotate over
syn keyword Conditional if else
syn keyword Repeat while break
syn keyword Boolean true false
syn match Operator "?"
syn match Operator "!"
syn match Operator "+"
syn match Operator "-"
syn match Operator "*"
syn match Operator "/"
syn match Operator "<"
syn match Operator ">"
syn match Operator ">="
syn match Operator "<="

syn match Comment "//.*$"
syn match Number "\v\d+"
syn match Float "\v\d+\.\d+"
syn region String start=/"/ end=/"/
