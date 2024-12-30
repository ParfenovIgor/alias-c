//* memory

//* _memcpy
//* Copies the data of length `sz` bytes from `src` pointer to `dest` pointer.
proto ._memcpy(dest #1I, src #1I, sz #I) -> #1I

//* _memmove
//* Copies the data of length `sz` bytes from `src` pointer to `dest` pointer. Handles possible overlapping
proto ._memmove(dest #1I, src #1I, sz #I) -> #1I

//* _memset
//* Sets the data of length `count` bytes to `dest` pointer with value `ch`. Important: only the lowest byte in `ch` argument is used.
proto ._memset(dest #1I, ch #I, count #I) -> #1I
