
import sys
import os
from pathlib import Path

ProjectDir = Path(sys.argv[1])
OutputDir = Path(sys.argv[2])

out = str(OutputDir.absolute()) + "\\rsc\\"
root = str(ProjectDir.absolute()) + "\\..\\rsc\\"

print("xcopy " + root + " " + out + " /Y /H /E /D")
os.system("xcopy " + root + " " + out + " /Y /H /E /D")