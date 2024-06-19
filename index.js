const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const { exec } = require('child_process');
const fs = require('fs');

const app = express();
const PORT = 5000;

app.use(cors());
app.use(bodyParser.json());

app.post('/execute', (req, res) => {
  const { numProcesses, arrivalTimes, burstTimes, priorities, timeQuantum } = req.body;

  // Log received data
  console.log('Received data:', req.body);

  // Prepare the input data for the C++ program
  const inputData = `${numProcesses}\n${arrivalTimes.join(' ')}\n${burstTimes.join(' ')}\n${priorities.join(' ')}\n${timeQuantum}\n`;

  // Log input data
  console.log('Input data for C++ program:', inputData);

  // Write the input data to a file
  fs.writeFileSync('input.txt', inputData);

  // Compile the C++ program
  exec('g++ -o a.out your_script.cpp', (compileError, compileStdout, compileStderr) => {
    if (compileError) {
      console.error('Compilation error:', compileStderr);
      return res.status(500).json({ output: compileStderr });
    }

    // Execute the compiled C++ program
    exec('a.out < input.txt', (execError, execStdout, execStderr) => {
      if (execError) {
        console.error('Execution error:', execStderr);
        return res.status(500).json({ output: execStderr });
      }
      res.json({ output: execStdout });
    });
  });
});

app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
