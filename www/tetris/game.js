// game.js

// Constants
const canvas = document.getElementById('game');
const context = canvas.getContext('2d');
const holdCanvas = document.getElementById('hold');
const holdContext = holdCanvas.getContext('2d');
const nextCanvases = document.querySelectorAll('.next-piece');
const nextContexts = Array.from(nextCanvases).map(c => c.getContext('2d'));
const grid = 32;
const colors = {
  'I': 'cyan',
  'O': 'yellow',
  'T': 'purple',
  'S': 'green',
  'Z': 'red',
  'J': 'blue',
  'L': 'orange'
};

// Game Variables
let playfield = [];
for (let row = -2; row < 20; row++) {
  playfield[row] = [];
  for (let col = 0; col < 10; col++) {
    playfield[row][col] = 0;
  }
}

const tetrominos = {
  'I': [
    [0,0,0,0],
    [1,1,1,1],
    [0,0,0,0],
    [0,0,0,0]
  ],
  'J': [
    [1,0,0],
    [1,1,1],
    [0,0,0],
  ],
  'L': [
    [0,0,1],
    [1,1,1],
    [0,0,0],
  ],
  'O': [
    [1,1],
    [1,1],
  ],
  'S': [
    [0,1,1],
    [1,1,0],
    [0,0,0],
  ],
  'Z': [
    [1,1,0],
    [0,1,1],
    [0,0,0],
  ],
  'T': [
    [0,1,0],
    [1,1,1],
    [0,0,0],
  ]
};

let tetrominoSequence = [];
let nextSequence = [];
generateSequence();
generateNextSequence();
let tetromino = getNextTetromino();
let hold = null;
let canHold = true;
let count = 0;
let rAF = null;
let gameOver = false;

// Generate a new tetromino sequence using the "bag" system
function generateSequence() {
  const sequence = ['I', 'J', 'L', 'O', 'S', 'T', 'Z'];
  while (sequence.length) {
    const rand = getRandomInt(0, sequence.length - 1);
    const name = sequence.splice(rand, 1)[0];
    tetrominoSequence.push(name);
  }
}

// Generate next sequence for previews
function generateNextSequence() {
  while (nextSequence.length < 3) { // Show next 3 pieces
    if (tetrominoSequence.length === 0) {
      generateSequence();
    }
    nextSequence.push(tetrominoSequence.pop());
  }
}

// Get the next tetromino
function getNextTetromino() {
  if (tetrominoSequence.length === 0) {
    generateSequence();
    generateNextSequence();
  }

  const name = nextSequence.shift();
  generateNextSequence();
  const matrix = tetrominos[name];
  const col = Math.floor(playfield[0].length / 2) - Math.ceil(matrix[0].length / 2);
  const row = name === 'I' ? -1 : -2;

  return {
    name: name,
    matrix: matrix,
    row: row,
    col: col
  };
}

// Rotate matrix 90 degrees
function rotate(matrix) {
  const N = matrix.length - 1;
  const result = matrix.map((row, i) =>
    row.map((val, j) => matrix[N - j][i])
  );
  return result;
}

// Check if move is valid
function isValidMove(matrix, cellRow, cellCol) {
  for (let row = 0; row < matrix.length; row++) {
    for (let col = 0; col < matrix[row].length; col++) {
      if (matrix[row][col]) {
        const newRow = cellRow + row;
        const newCol = cellCol + col;
        if (
          newCol < 0 ||
          newCol >= playfield[0].length ||
          newRow >= playfield.length ||
          (newRow >= 0 && playfield[newRow][newCol])
        ) {
          return false;
        }
      }
    }
  }
  return true;
}

// Place tetromino on the playfield
function placeTetromino() {
  for (let row = 0; row < tetromino.matrix.length; row++) {
    for (let col = 0; col < tetromino.matrix[row].length; col++) {
      if (tetromino.matrix[row][col]) {
        const newRow = tetromino.row + row;
        const newCol = tetromino.col + col;

        if (newRow < 0) {
          return showGameOver();
        }

        playfield[newRow][newCol] = tetromino.name;
      }
    }
  }

  // Clear lines
  for (let row = playfield.length -1; row >=0; ) {
    if (playfield[row].every(cell => !!cell)) {
      for (let r = row; r > 0; r--) {
        playfield[r] = [...playfield[r-1]];
      }
      playfield[0] = Array(10).fill(0);
    } else {
      row--;
    }
  }

  tetromino = getNextTetromino();
  canHold = true;
}

// Show Game Over
function showGameOver() {
  cancelAnimationFrame(rAF);
  gameOver = true;

  context.fillStyle = 'black';
  context.globalAlpha = 0.75;
  context.fillRect(0, canvas.height / 2 - 30, canvas.width, 60);

  context.globalAlpha = 1;
  context.fillStyle = 'white';
  context.font = '36px monospace';
  context.textAlign = 'center';
  context.textBaseline = 'middle';
  context.fillText('GAME OVER!', canvas.width / 2, canvas.height / 2);
}

// Quick Drop Function
function quickDrop() {
  while (isValidMove(tetromino.matrix, tetromino.row + 1, tetromino.col)) {
    tetromino.row++;
  }
  placeTetromino();
}

// Hold Function
function holdPiece() {
  if (!canHold) return;
  canHold = false;

  if (hold) {
    const temp = hold;
    hold = tetromino;
    tetromino = temp;
    tetromino.row = tetromino.name === 'I' ? -1 : -2;
    tetromino.col = Math.floor(playfield[0].length / 2) - Math.ceil(tetromino.matrix[0].length / 2);
    if (!isValidMove(tetromino.matrix, tetromino.row, tetromino.col)) {
      showGameOver();
    }
  } else {
    hold = tetromino;
    tetromino = getNextTetromino();
  }
}

// Draw Functions
function drawPlayfield() {
  for (let row = 0; row < 20; row++) {
    for (let col = 0; col < 10; col++) {
      if (playfield[row][col]) {
        const name = playfield[row][col];
        context.fillStyle = colors[name];
        context.fillRect(col * grid, row * grid, grid-1, grid-1);
      }
    }
  }
}

function drawTetromino() {
  if (tetromino) {
    context.fillStyle = colors[tetromino.name];
    for (let row = 0; row < tetromino.matrix.length; row++) {
      for (let col = 0; col < tetromino.matrix[row].length; col++) {
        if (tetromino.matrix[row][col]) {
          const x = (tetromino.col + col) * grid;
          const y = (tetromino.row + row) * grid;
          context.fillRect(x, y, grid-1, grid-1);
        }
      }
    }
  }
}

function drawHold() {
  holdContext.clearRect(0, 0, holdCanvas.width, holdCanvas.height);
  if (hold) {
    const matrix = hold.matrix;
    const name = hold.name;
    holdContext.fillStyle = colors[name];
    const offsetX = (holdCanvas.width - matrix[0].length * grid) / 2;
    const offsetY = (holdCanvas.height - matrix.length * grid) / 2;
    for (let row = 0; row < matrix.length; row++) {
      for (let col = 0; col < matrix[row].length; col++) {
        if (matrix[row][col]) {
          holdContext.fillRect(
            offsetX + col * grid,
            offsetY + row * grid,
            grid - 2,
            grid - 2
          );
        }
      }
    }
  }
}

function drawNext() {
  nextContexts.forEach((ctx, index) => {
    ctx.clearRect(0, 0, holdCanvas.width, holdCanvas.height);
    const name = nextSequence[index];
    if (name) {
      const matrix = tetrominos[name];
      ctx.fillStyle = colors[name];
      const offsetX = (holdCanvas.width - matrix[0].length * grid) / 2;
      const offsetY = (holdCanvas.height - matrix.length * grid) / 2;
      for (let row = 0; row < matrix.length; row++) {
        for (let col = 0; col < matrix[row].length; col++) {
          if (matrix[row][col]) {
            ctx.fillRect(
              offsetX + col * grid,
              offsetY + row * grid,
              grid - 2,
              grid - 2
            );
          }
        }
      }
    }
  });
}

function draw() {
  context.clearRect(0, 0, canvas.width, canvas.height);
  drawPlayfield();
  drawTetromino();
  drawHold();
  drawNext();
}

// Game Loop
function loop() {
  rAF = requestAnimationFrame(loop);
  draw();

  // Tetromino falls every 35 frames
  if (++count > 35) {
    tetromino.row++;
    count = 0;

    if (!isValidMove(tetromino.matrix, tetromino.row, tetromino.col)) {
      tetromino.row--;
      placeTetromino();
    }
  }
}

// Event Listeners
document.addEventListener('keydown', function(e) {
  if (gameOver) return;

  switch(e.code) {
    case 'ArrowLeft':
      {
        const col = tetromino.col - 1;
        if (isValidMove(tetromino.matrix, tetromino.row, col)) {
          tetromino.col = col;
        }
        break;
      }
    case 'ArrowRight':
      {
        const col = tetromino.col + 1;
        if (isValidMove(tetromino.matrix, tetromino.row, col)) {
          tetromino.col = col;
        }
        break;
      }
    case 'ArrowUp':
      {
        const rotated = rotate(tetromino.matrix);
        if (isValidMove(rotated, tetromino.row, tetromino.col)) {
          tetromino.matrix = rotated;
        }
        break;
      }
    case 'ArrowDown':
      {
        const row = tetromino.row + 1;
        if (!isValidMove(tetromino.matrix, row, tetromino.col)) {
          tetromino.row = row - 1;
          placeTetromino();
        } else {
          tetromino.row = row;
        }
        break;
      }
    case 'Space':
      {
        quickDrop();
        break;
      }
    case 'KeyC':
      {
        holdPiece();
        break;
      }
    default:
      break;
  }
});

// Utility Functions
function getRandomInt(min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);

  return Math.floor(Math.random() * (max - min + 1)) + min;
}

// Start the game
rAF = requestAnimationFrame(loop);
