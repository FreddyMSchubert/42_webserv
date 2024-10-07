let score = 0;

function addScore()
{
	score++;
	localStorage.setItem("score", score);
	document.getElementsByClassName("score")[0].innerHTML = score;

	const cookieWrapper = document.getElementsByClassName("cookie-wrapper")[0];
	cookieWrapper.classList.remove("animate");
	void cookieWrapper.offsetWidth; // Force a reflow to reset animation
	cookieWrapper.classList.add("animate");
	setTimeout(() => cookieWrapper.classList.remove("animate"), 300);
}

document.getElementsByClassName("btn")[0].addEventListener("click", addScore);

document.addEventListener('DOMContentLoaded', function() {
	
	if (localStorage.getItem("score"))
		score = parseInt(localStorage.getItem("score"));
	else
		localStorage.setItem("score", 0);
	document.getElementsByClassName("score")[0].innerHTML = score;
});

