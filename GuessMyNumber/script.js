"use sctrict";

let secretNumber = Math.trunc(Math.random() * 100);
console.log(secretNumber);

let attempts = 10;
let highScore = 0;

document.querySelector(".check").addEventListener("click", function () {
  const guess = Number(document.querySelector(".guess").value);

  // No input
  if (!guess) {
    document.querySelector(".message").textContent = "⛔ No Number";
  }
  //Correct number guessed
  else if (guess === secretNumber) {
    document.querySelector(".message").textContent = "🥳 Correct Number!!";
    document.querySelector(".number").textContent = secretNumber;
    document.querySelector("body").style.backgroundColor = "#60b347";
    document.querySelector(".number").style.width = "30rem";

    if (attempts > highScore) {
      highScore = attempts;
    }
    document.querySelector(".highscore").textContent = highScore;
  }
  //when guess number is wrong
  else if (guess != secretNumber) {
    if (attempts > 1) {
      document.querySelector(".message").textContent =
        guess > secretNumber ? "📈 Too High..." : "📉 Too Low...";
      attempts--;
      document.querySelector(".score").textContent = attempts;
    } else {
      document.querySelector(".message").textContent = "😢 You Lost The Game.";
      document.querySelector(".score").textContent = 0;
      document.querySelector(".number").textContent = secretNumber;
    }
  }
});

//again button
document.querySelector(".again").addEventListener("click", function () {
  attempts = 10;
  secretNumber = Math.trunc(Math.random() * 100);
  console.log(secretNumber);
  document.querySelector("body").style.backgroundColor = "#222";
  document.querySelector(".message").textContent = "Start Guessing ...";
  document.querySelector(".score").textContent = attempts;
  document.querySelector(".number").textContent = "?";
  document.querySelector(".number").style.width = "15rem";
  document.querySelector(".guess").value = "";
});
