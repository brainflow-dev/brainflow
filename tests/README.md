# Brainflow Tests

## Instructions to run tests

You need to install emulator package first
```
cd {project_dir}/emulator
python setup.py install
```

### Run Python tests
```
cd {project_dir}/python-package
pip install -e .
# to test it on Linux machine
python3 {project_dir}/emulator/brainflow_emulator/cython_linux.py python3 {project_dir}/tests/python/brainflow_get_data.py
# to test it on Windows machine
python3 {project_dir}/emulator/brainflow_emulator/cython_windows.py python3 {project_dir}/tests/python/brainflow_get_data.py
```

### Run Java test
```
cd {project_dir}\java-package
# I dont add jar to repo, so you will need to build it
mvn package
cd {project_dir}/tests/java
javac -classpath {project_dir}\java-package\brainflow\target\brainflow-java-jar-with-dependencies.jar BrainFlowTest.java
python {project_dir}\emulator\brainflow_emulator\cython_windows.py java -classpath .;{project_dir}\java-package\brainflow\target\brainflow-java-jar-with-dependencies.jar BrainFlowTest
```
