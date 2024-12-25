include altlib."memory.al"
include altlib."stdio.al"
include altlib."stdlib.al"

typedef Vector := #S{
    data: #1I,
    size: #I,
    reserved: #I
};

func ^.vector_init() -> #1Vector {
    def this #1Vector := _malloc($#Vector) as #1Vector
    this->size& <- 0 
    this->reserved& <- 10
    this->data& <- _malloc(this->reserved * $#I)
    return this
}

func ^#1Vector.push(this #1Vector, x #I) -> #V {
    eval if (this->size = this->reserved) {
        def buffer #1I := _malloc(this->reserved * 2 * $#I)
        eval puti_(this->data as #I)
        eval _memcpy(buffer, this->data, this->size * $#I)
        eval _free(this->data)
        this->data& <- buffer
        this->reserved& <- this->reserved * 2
    }
    this->data[this->size]& <- x
    this->size& <- this->size + 1
}

func ^#1Vector.pop(this #1Vector) -> #I {
    return if (this->size = 0) { return 0 }
    else {
        this->size& <- this->size - 1
        return 1
    }
}

func ^#1Vector.get(this #1Vector, x #I) -> #I {
    return this->data[x]
}
