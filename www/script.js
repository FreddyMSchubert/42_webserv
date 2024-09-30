let score = 0;

function addScore()
{
	score++;
	document.cookie = "score=" + score;
	document.getElementsByClassName("score")[0].innerHTML = score;
}

document.getElementsByClassName("btn")[0].addEventListener("click", addScore);

document.addEventListener('DOMContentLoaded', function() {
	
	if (document.cookie == "")
		document.cookie = "score=0";
	
	let cookie = document.cookie.split("=");
	if(cookie[0] == "score")
	{
		score = parseInt(cookie[1]);
		document.getElementsByClassName("score")[0].innerHTML = score;
	}
});

