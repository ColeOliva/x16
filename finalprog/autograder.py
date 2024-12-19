import json
import os

autograding = {
  "tests": [
    {
      "name": "Test 1. Test virtual address to page number",
      "setup": "",
      "run": "make test-to-page",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 1
    },
    {
      "name": "Test 2. Test virtual address to offset",
      "setup": "",
      "run": "make test-to-offset",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 1
    },
    {
      "name": "Test 3. Test page and offset to address",
      "setup": "",
      "run": "make test-to-address",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 1
    },
    {
      "name": "Test 4. Test valid lookup that finds a physical address",
      "setup": "",
      "run": "make test-lookup-found",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 1
    },
    {
      "name": "Test 5. Test invalid virtual address lookup in TLB",
      "setup": "",
      "run": "make test-lookup-not-found",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 1
    },
    {
      "name": "Test 6. Test ordering of TLB on access",
      "setup": "",
      "run": "make test-order",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 1
    },
    {
      "name": "Test 7. Test load of TLB from page table",
      "setup": "",
      "run": "make test-load",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 2
    },
    {
      "name": "Test 8. Test lookup and load",
      "setup": "",
      "run": "make test-lookup-and-load",
      "input": "",
      "output": "",
      "comparison": "included",
      "timeout": 5,
      "points": 2
    },
  ]
}


def runtest(test):
    print(test["name"] + ", " + str(test["points"]) + " points")
    if test["setup"].startswith("make"):
        os.system(test["setup"])
    if os.system(test["run"] + " > out") != 0:
        return 0
    if test["output"] != "":
        grepcmd = "grep \"" + test["output"] + "\" out"
        if os.system(grepcmd) != 0:
            return 0
    return test["points"]

def main():
    score = 0
    for test in autograding["tests"]:
        score += runtest(test)
    print("Final score=" + str(score))

if __name__ == "__main__":
    main()
