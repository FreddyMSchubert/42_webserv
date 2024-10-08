let score = 0;
let clicks = [];
let autoClicks = [];
let maxCPS = 0;
let maxEverCPS = 0;
let automations = [
	{ name: 'Byte Digger', cost: 15, cps: 0.1, count: 0 },
	{ name: 'Incompetent Junior', cost: 100, cps: 1, count: 0 },
	{ name: 'Data Mill', cost: 1100, cps: 8, count: 0 },
	{ name: 'Master of Recursivity', cost: 12000, cps: 47, count: 0 },
	{ name: 'Handler of Exception', cost: 130000, cps: 260, count: 0 },
	{ name: 'Algorithm Optimizer', cost: 1400000, cps: 1400, count: 0 },
	{ name: 'Legacy Refactorer', cost: 20000000, cps: 7800, count: 0 },
	{ name: 'System Architect', cost: 330000000, cps: 44000, count: 0 },
	{ name: 'Quantum Debugger', cost: 5100000000, cps: 260000, count: 0 },
	{ name: 'Sentient DevOps', cost: 75000000000, cps: 1600000, count: 0 },
	{ name: 'The Singularity', cost: 1000000000000, cps: 100000000, count: 0 }
];
let savedClicks = 0;

function addScore(increment = 1, humanMade = false)
{
	let previousSavedClicks = savedClicks;
	savedClicks += increment;
	let fullClickHappened = Math.floor(savedClicks) > Math.floor(previousSavedClicks);

	// score
	score += increment;
	localStorage.setItem("score", score);
	document.getElementsByClassName("score")[0].innerHTML = Math.round(score).toLocaleString();

	if (!fullClickHappened)
		return;

	// particles
	const fallingParticle = document.createElement("img");
	fallingParticle.classList.add("falling-particle");
	fallingParticle.src = "assets/42logo.png";

	if (humanMade)
		fallingParticle.style.filter = `invert(1) sepia(1) saturate(5) hue-rotate(${Math.random() * 360}deg) brightness(1)`;
	else
		fallingParticle.style.filter = `brightness(0)`;

	const sizeMultiplier = getSizeMultiplier();
	let particleScalePx = 50;
	if (humanMade)
		particleScalePx *= sizeMultiplier;
	else if (increment > 1)
		particleScalePx *= increment * 2 / increment;
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
	if (humanMade)
		clicks.push(Date.now());
	else
		autoClicks.push(Date.now());
	updateCPS();

	// button animation
	if (!humanMade)
		return;
	const btnWrapper = document.getElementsByClassName("btn-wrapper")[0];
	btnWrapper.classList.remove("animate");
	void btnWrapper.offsetWidth; // Force a reflow to reset animation
	btnWrapper.classList.add("animate");
	setTimeout(() => btnWrapper.classList.remove("animate"), 300);
}
document.getElementsByClassName("btn")[0].addEventListener("click", event => addScore(1, true));
document.addEventListener("keyup", event => {
	if (event.key === "Enter" || event.key === " ")
		addScore(1, true);
});

function getSizeMultiplier()
{
	const cps = clicks.filter(t => Date.now() - t <= 1000).length;
	return 1 + (cps * 1.5);
}

function updateCPS()
{
	autoClicks = autoClicks.filter(t => Date.now() - t <= 1000);
	clicks = clicks.filter(t => Date.now() - t <= 1000);
	if (clicks.length > maxCPS)
		maxCPS = clicks.length;
	if (maxCPS > maxEverCPS)
	{
		maxEverCPS = maxCPS;
		localStorage.setItem("maxCPS", maxEverCPS);
	}
	document.getElementsByClassName("cps")[0].innerHTML = clicks.length.toLocaleString();
	document.getElementsByClassName("maxcps")[0].innerHTML = maxCPS.toLocaleString();
	document.getElementsByClassName("maxevercps")[0].innerHTML = maxEverCPS.toLocaleString();
	document.getElementsByClassName("autocps")[0].innerHTML = getAutoCPS().toFixed(1).toLocaleString();
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
	setInterval(autoScore, 10);
	initializeAutomation();
});

/* ---- AUTOMATIONS ----- */

function initializeAutomation()
{
	automations.forEach((automation, index) => {
		if (localStorage.getItem(`automation${index}cost`))
			automation.cost = localStorage.getItem(`automation${index}cost`) || automation.cost;
		if (localStorage.getItem(`automation${index}count`))
			automation.count = localStorage.getItem(`automation${index}count`) || automation.count;
	});
	writeAutomationData();
}

function writeAutomationData()
{
	const list = document.getElementById("automation-items");
	list.innerHTML = "";
	automations.forEach((automation, index) => {
		const item = document.createElement("li");
		item.className = "automation-item";
		item.innerHTML = `
			${automation.name} - Cost: ${Number(Number(automation.cost).toFixed(2)).toLocaleString()}, CPS: ${automation.cps.toLocaleString()}, Owned: ${automation.count.toLocaleString()}
		`;
		item.addEventListener("click", () => buyAutomation(index));
		list.appendChild(item);
	});
}

// Buy automation
function buyAutomation(index)
{
	const automation = automations[index];
	if (score >= automation.cost)
	{
		score -= automation.cost;
		automation.count++;
		automation.cost = automation.cost * 1.15;
		localStorage.setItem("score", score);
		localStorage.setItem(`automation${index}cost`, automation.cost);
		localStorage.setItem(`automation${index}count`, automation.count);
		writeAutomationData();
		document.getElementsByClassName("score")[0].innerHTML = score;
	}
}

function getAutoCPS()
{
	let autoIncrement = 0;
	for (let i = 0; i < automations.length; i++)
		autoIncrement += automations[i].cps * automations[i].count;
	return autoIncrement;
}

function autoScore()
{
	const autoIncrement = getAutoCPS() / 100;
	if (autoIncrement > 0)
		addScore(autoIncrement);
}
