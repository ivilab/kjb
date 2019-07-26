

s/ctype_ex/ctype/

/^\.TH /{
s/^\.TH "\([^"]*\)" \([^ ]*\) .*/.ds [H \1\\|(\\|\2\\|)/
a\
.ds [D UNIX Programmer's Manual\
.po +1i\
.lt -1.5i\
.tl @\\*([H@\\*([D@\\*([H@\
.lt +1.5i\
.po -1i\
.RS +1i\
.nr CL \\n(.l-0.5i
}

/^\.S[SH] /{
i\
.br\
.ne 3\
.RE\
.po +1i
a\
.po -1i\
.RS +1i\
.ll \\n(CLu
}

/^.TP/{
n
a\
.ll \\n(CLu
}

$a\
.ll \\n(CLu+0.5i\
.RE

