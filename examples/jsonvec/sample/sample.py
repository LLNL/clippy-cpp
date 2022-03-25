from clippy import clippy_import

clippy_import("/PATH/TO/clippy-cpp/build/examples/jsonvec")

vec = JsonLines("./jltest") 

vec.importFile("/p/lustre3/llamag/reddit/comments/RC_2007-01")

q = vec[vec.row.score > 5]
q.eval()

q = vec[vec.rows.score > 5, vec.rows.controversiality == 1]
q.eval()

-- but NOT
q = vec[vec.rows.score > 5 and vec.rows.controversiality == 1]
q.eval() -- the left-hand-side condition gets dropped on the Python side

