Import("env", "projenv")

# access to global build environment
print(env)

# access to project build environment (is used source files in "src" folder)
print(projenv)

print("#"*40)
print("#"*40)
print("#"*40)


def after_build(source, target, env):
    print("after_build")
    # do some actions

env.AddPostAction("buildprog", after_build)
