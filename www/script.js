let score = 0;
let clicks = [];
let autoClicks = [];
let maxCPS = 0;
let maxEverCPS = 0;
let savedClicks = 0;
let automations = [
	{ name: 'Byte Digger', cost: 15, cps: 0.1, count: 0, efficiency: 1 },
	{ name: 'Incompetent Junior', cost: 100, cps: 1, count: 0, efficiency: 1 },
	{ name: 'Data Mill', cost: 1100, cps: 8, count: 0, efficiency: 1 },
	{ name: 'Master of Recursivity', cost: 12000, cps: 47, count: 0, efficiency: 1 },
	{ name: 'Handler of Exception', cost: 130000, cps: 260, count: 0, efficiency: 1 },
	{ name: 'Algorithm Optimizer', cost: 1400000, cps: 1400, count: 0, efficiency: 1 },
	{ name: 'Legacy Refactorer', cost: 20000000, cps: 7800, count: 0, efficiency: 1 },
	{ name: 'System Architect', cost: 330000000, cps: 44000, count: 0, efficiency: 1 },
	{ name: 'Quantum Debugger', cost: 5100000000, cps: 260000, count: 0, efficiency: 1 },
	{ name: 'Sentient DevOps', cost: 75000000000, cps: 1600000, count: 0, efficiency: 1 },
	{ name: 'The Singularity', cost: 1000000000000, cps: 100000000, count: 0, efficiency: 1 }
];
let upgrades = [
	{ name: 'Carpal Tunnel Prevention', cost: 100, unlock_amount: 1, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention II', cost: 500, unlock_amount: 1, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention III', cost: 10000, unlock_amount: 10, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention IV', cost: 100000, unlock_amount: 25, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention V', cost: 10000000, unlock_amount: 50, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention VI', cost: 100000000, unlock_amount: 100, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention VII', cost: 1000000000, unlock_amount: 150, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Carpal Tunnel Prevention VIII', cost: 10000000000, unlock_amount: 200, unlock_automation: 0, result: "click_eff", factor: 2, owned: false },
	{ name: 'Buff Diggers', cost: 9990, unlock_amount: 10, unlock_automation: 0, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Juniors', cost: 66600, unlock_amount: 10, unlock_automation: 1, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Mills', cost: 750000, unlock_amount: 10, unlock_automation: 2, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Masters', cost: 8000000, unlock_amount: 10, unlock_automation: 3, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Handlers', cost: 90000000, unlock_amount: 10, unlock_automation: 4, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Optimizers', cost: 1000000000, unlock_amount: 10, unlock_automation: 5, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Refactorers', cost: 10000000000, unlock_amount: 10, unlock_automation: 6, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Architects', cost: 100000000000, unlock_amount: 10, unlock_automation: 7, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Debuggers', cost: 1000000000000, unlock_amount: 10, unlock_automation: 8, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff DevOpss', cost: 10000000000000, unlock_amount: 10, unlock_automation: 9, result: "building_eff", factor: 2, owned: false },
	{ name: 'Buff Singularities', cost: 100000000000000, unlock_amount: 10, unlock_automation: 10, result: "building_eff", factor: 2, owned: false },
	{ name: 'Godlike Juniors', cost: 99900, unlock_amount: 50, unlock_automation: 0, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Juniors', cost: 666000, unlock_amount: 50, unlock_automation: 1, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Mills', cost: 75000000, unlock_amount: 50, unlock_automation: 2, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Masters', cost: 8000000000, unlock_amount: 50, unlock_automation: 3, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Handlers', cost: 900000000000, unlock_amount: 50, unlock_automation: 4, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Optimizers', cost: 100000000000000, unlock_amount: 50, unlock_automation: 5, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Refactorers', cost: 1000000000000000, unlock_amount: 50, unlock_automation: 6, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Architects', cost: 1000000000000000, unlock_amount: 50, unlock_automation: 7, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Debuggers', cost: 1000000000000000, unlock_amount: 50, unlock_automation: 8, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike DevOpss', cost: 1000000000000000, unlock_amount: 50, unlock_automation: 9, result: "building_eff", factor: 3, owned: false },
	{ name: 'Godlike Singularities', cost: 1000000000000000, unlock_amount: 50, unlock_automation: 10, result: "building_eff", factor: 3, owned: false },
];

// upgrade stats
let clickEfficiency = 1;

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
	for (let i = 0; i < increment - 1 && humanMade; i++)
	{
		document.getElementsByClassName("falling-particles")[0].appendChild(fallingParticle.cloneNode());
	}
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
document.getElementsByClassName("btn")[0].addEventListener("click", event => addScore(clickEfficiency, true));
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
	initializeUpgrades();
});

/* ---- AUTOMATIONS ----- */

function initializeAutomation()
{
	automations.forEach((automation, index) => {
		if (localStorage.getItem(`automation${index}cost`))
			automation.cost = localStorage.getItem(`automation${index}cost`) || automation.cost;
		if (localStorage.getItem(`automation${index}count`))
			automation.count = localStorage.getItem(`automation${index}count`) || automation.count;
		if (localStorage.getItem(`automation${index}efficiency`))
			automation.efficiency = localStorage.getItem(`automation${index}efficiency`) || automation.efficiency;
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
		if (automation.efficiency > 1)
			item.innerHTML += ` Efficiency: ${automation.efficiency}x`;
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
	initializeUpgrades();
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

/* ---- UPGRADES ----- */

function initializeUpgrades()
{
	upgrades.forEach((upgrade, index) => {
		if (localStorage.getItem(`upgrade${index}owned`))
			upgrade.owned = localStorage.getItem(`upgrade${index}owned`) === "true";
	});
	clickEfficiency = Number(localStorage.getItem("clickEfficiency")) || 1;
	writeUpgradeData();
}

function writeUpgradeData()
{
	const list = document.getElementById("upgrade-list");
	list.innerHTML = "";
	upgrades.forEach((upgrade, index) => {
		if (automations[upgrade.unlock_automation].count < upgrade.unlock_amount || upgrade.owned)
			return;
		const item = document.createElement("li");
		item.className = "upgrade-item";
		item.innerHTML = `
			${upgrade.name} - Cost: ${Number(Number(upgrade.cost).toFixed(2)).toLocaleString()}, Owned: ${upgrade.owned}
		`;
		item.addEventListener("click", () => buyUpgrade(index));
		list.appendChild(item);
	});
}

function buyUpgrade(index)
{
	const upgrade = upgrades[index];
	if (score >= upgrade.cost)
	{
		score -= upgrade.cost;
		upgrade.owned = true;
		localStorage.setItem("score", score);
		localStorage.setItem(`upgrade${index}owned`, upgrade.owned);
		writeUpgradeData();
		document.getElementsByClassName("score")[0].innerHTML = score;

		switch (upgrade.result)
		{
			case "click_eff":
				clickEfficiency *= upgrade.factor;
				localStorage.setItem("clickEfficiency", clickEfficiency);
				break;
			case "building_eff":
				automations[upgrade.unlock_automation].efficiency *= upgrade.factor;
				localStorage.setItem(`automation${upgrade.unlock_automation}efficiency`, automations[upgrade.unlock_automation].efficiency);
				writeAutomationData();
				break;
		}
	}
}

function reset()
{
	localStorage.clear();
	location.reload();
	score = 0;
	clicks = [];
	autoClicks = [];
	maxCPS = 0;
	maxEverCPS = 0;
	savedClicks = 0;
}
