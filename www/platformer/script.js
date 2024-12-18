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

obstacle.addEventListener('animationend', () => {
	obstacle.style.animation = 'none';
	obstacle.offsetHeight;
	score++;
	scoreElement.textContent = score;
	if (score > highScore)
	{
		highScore = score;
		highScoreElement.textContent = highScore;
		localStorage.setItem('highScore', highScore);
	}
	const delay = Math.random() * 0.5 * 1000;
	setTimeout(() => {
		obstacle.style.animation = `obstacle-move ${((Math.random() * 1.25) + 1).toFixed(2)}s linear`;
	}, delay);
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
		score = 0;
		scoreElement.textContent = score;
	}
}, 100);
