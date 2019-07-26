s!$(privlib)!/usr/local/lib/c2man!
/^.so *example.h$/{
r example.h
d
}

/^.so *example.inc$/{
r example.inc
d
}

/^.so *ctype_ex.h$/{
r ctype_ex.h
d
}

/^.so *ctype_ex.inc$/{
r ctype_ex.inc
d
}
