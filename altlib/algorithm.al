//* algorithm

//* min_
//* Gets two integer values and returns the minimum one.
func ^.min_(a #I, b #I) -> #I
    if (a < b) a else b

//* max_
//* Gets two integer values and returns the maximum one.
func ^.max_(a #I, b #I) -> #I
    if (a < b) b else a
