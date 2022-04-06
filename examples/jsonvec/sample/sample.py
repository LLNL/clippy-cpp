from clippy import clippy_import

clippy_import("/PATH/TO/clippy-cpp/build/examples/jsonvec")

vec = JsonLines("./jltest") 

vec.importFile("/p/lustre3/llamag/reddit/comments/RC_2007-01")

q = vec[vec.keys.score > 5]
q.eval()

q = vec[vec.keys.score > 5, vec.keys.controversiality == 1]
q.eval()

# alternative 1:
q = vec[vec.keys.score > 5][vec.keys.controversiality == 1]
q.eval()

# alternative 2:
q = vec[vec.keys.score > 5 & vec.keys.controversiality == 1]
q.eval()

# but NOT
q = vec[vec.keys.score > 5 and vec.rows.controversiality == 1]
q.eval() -- the left-hand-side condition gets dropped on the Python side

# The reason is that "and" is the logical short circuit operator that cannot
# be overloaded.


### Thoughts on:

## string find: a in b calls b.__contains__(a)
q = vec["abc" in vec.keys.name]

# thus probably not: 
q = vec[vec.keys.name in "abcdefg"]

## regex
q = vec[vec.regex("[a..z]*") in vec.keys.name]


