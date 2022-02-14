
# Sample query demonstrating how a freestanding function
# can return a clippy object.

# import clippy functionality
from clippy import ooclippy
from clippy import clippy_import

# load class description
processDirectory("BASE/clippy-cpp/build/examples/oo-dataframe")

# load freestanding function (from a different directory) into namespace Cities
clippy_import("BASE/git/clippy-cpp/build/examples/dataframe-load", "Cities")

#
# following code assumes that the city dataframe already exists

# create a clippy object from a freestanding function in the new namespace
df = Cities.load(".", "cities")

# use the new object
df.metadata()


