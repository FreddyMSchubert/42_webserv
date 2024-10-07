let score = 0;

function addScore()
{
	score++;
	localStorage.setItem("score", score);
	document.getElementsByClassName("score")[0].innerHTML = score;

	const fallingParticle = document.createElement("img");
	fallingParticle.classList.add("falling-particle");
	fallingParticle.src = "assets/42logo.png";
	const particleWidthPx = 50;
	const maxLeftPx = window.innerWidth - particleWidthPx;
	fallingParticle.style.left = `${Math.random() * maxLeftPx}px`;
	fallingParticle.style.top = '-50px';
	document.getElementsByClassName("falling-particles")[0].appendChild(fallingParticle);
	setTimeout(() => {
		fallingParticle.remove();
	}, 3000);

	const btnWrapper = document.getElementsByClassName("btn-wrapper")[0];
	btnWrapper.classList.remove("animate");
	void btnWrapper.offsetWidth; // Force a reflow to reset animation
	btnWrapper.classList.add("animate");
	setTimeout(() => btnWrapper.classList.remove("animate"), 300);
}

document.getElementsByClassName("btn")[0].addEventListener("click", addScore);

document.addEventListener('DOMContentLoaded', function() {
	
	if (localStorage.getItem("score"))
		score = parseInt(localStorage.getItem("score"));
	else
		localStorage.setItem("score", 0);
	document.getElementsByClassName("score")[0].innerHTML = score;
});

