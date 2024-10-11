let player = document.getElementById('player');
let obstacle = document.getElementById('obstacle');
let scoreElement = document.getElementById('score');
let highScoreElement = document.getElementById('high-score');
let score = 0;
let highScore = localStorage.getItem('highScore') || 0;
highScoreElement.textContent = highScore;

let isJumping = false;

document.addEventListener('keydown', function(event) {
	if (event.code === 'Space' && !isJumping) {
		jump();
	}
});

function jump() {
	isJumping = true;
	let jumpHeight = 0;
	let upInterval = setInterval(() => {
		if (jumpHeight >= 100) {
			clearInterval(upInterval);
			let downInterval = setInterval(() => {
				if (jumpHeight <= 0) {
					clearInterval(downInterval);
					isJumping = false;
				}
				jumpHeight -= 5;
				player.style.bottom = 20 + jumpHeight + 'px';
			}, 20);
		}
		jumpHeight += 5;
		player.style.bottom = 20 + jumpHeight + 'px';
	}, 20);
}

let checkCollision = setInterval(() => {
	let playerRect = player.getBoundingClientRect();
	let obstacleRect = obstacle.getBoundingClientRect();

	if (
		playerRect.right > obstacleRect.left &&
		playerRect.left < obstacleRect.right &&
		playerRect.bottom > obstacleRect.top &&
		playerRect.top < obstacleRect.bottom
	) {
		alert('Game Over!');
		obstacle.style.animation = 'none';
		let finalScore = score;
		if (finalScore > highScore) {
			localStorage.setItem('highScore', finalScore);
			highScoreElement.textContent = finalScore;
		}
		score = 0;
		scoreElement.textContent = score;
		obstacle.style.animation = 'obstacle-move 2s linear infinite';
	} else {
		score++;
		scoreElement.textContent = score;
	}
}, 100);
