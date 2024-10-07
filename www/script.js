let score = 0;
let clicks = [];
let maxCPS = 0;
let maxEverCPS = 0;

function addScore()
{
	// score
	score++;
	localStorage.setItem("score", score);
	document.getElementsByClassName("score")[0].innerHTML = score;

	// particles
	const fallingParticle = document.createElement("img");
	fallingParticle.classList.add("falling-particle");
	fallingParticle.src = "assets/42logo.png";

	fallingParticle.style.filter = `invert(1) sepia(1) saturate(5) hue-rotate(${Math.random() * 360}deg) brightness(1)`;

	const sizeMultiplier = getSizeMultiplier();
	const particleScalePx = 50 * sizeMultiplier;
	const maxLeftPx = window.innerWidth - particleScalePx;

	fallingParticle.style.width = `${particleScalePx}px`;
	fallingParticle.style.height = `${particleScalePx}px`;
	fallingParticle.style.left = `${Math.random() * maxLeftPx}px`;
	fallingParticle.style.top = '-50px';

	document.getElementsByClassName("falling-particles")[0].appendChild(fallingParticle);
	setTimeout(() => {
		fallingParticle.remove();
	}, 3000);

	// cps
	clicks.push(Date.now());
	updateCPS();

	// button animation
	const btnWrapper = document.getElementsByClassName("btn-wrapper")[0];
	btnWrapper.classList.remove("animate");
	void btnWrapper.offsetWidth; // Force a reflow to reset animation
	btnWrapper.classList.add("animate");
	setTimeout(() => btnWrapper.classList.remove("animate"), 300);
}
document.getElementsByClassName("btn")[0].addEventListener("click", addScore);

function getSizeMultiplier()
{
	const cps = clicks.filter(t => Date.now() - t <= 1000).length;
	return 1 + (cps * 1.5);
}

function updateCPS()
{
	clicks = clicks.filter(t => Date.now() - t <= 1000);
	if (clicks.length > maxCPS)
		maxCPS = clicks.length;
	if (maxCPS > maxEverCPS)
	{
		maxEverCPS = maxCPS;
		localStorage.setItem("maxCPS", maxEverCPS);
	}
	document.getElementsByClassName("cps")[0].innerHTML = clicks.length;
	document.getElementsByClassName("maxcps")[0].innerHTML = maxCPS;
	document.getElementsByClassName("maxevercps")[0].innerHTML = maxEverCPS;
}

document.addEventListener('DOMContentLoaded', function() {
	
	if (localStorage.getItem("score"))
		score = parseInt(localStorage.getItem("score"));
	else
		localStorage.setItem("score", 0);
	if (localStorage.getItem("maxCPS"))
		maxEverCPS = parseInt(localStorage.getItem("maxCPS"));
	else
		localStorage.setItem("maxCPS", 0);
	document.getElementsByClassName("score")[0].innerHTML = score;
	setInterval(updateCPS, 100);
});

