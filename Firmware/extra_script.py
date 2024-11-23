Import("env")

import subprocess

def get_git_info():
    try:
        # Get current branch name
        branch = subprocess.check_output(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"],
            universal_newlines=True
        ).strip()

        # Get current commit hash
        commit = subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"], 
            universal_newlines=True
        ).strip()

        return branch, commit
    except:
        return "unknown", "unknown"

branch, commit = get_git_info()
env.Append(CPPDEFINES=[
    ("GIT_BRANCH", '\\"%s\\"' % branch),
    ("GIT_COMMIT", '\\"%s\\"' % commit)
])