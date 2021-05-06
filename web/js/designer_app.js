"use strict";

var RAVELER = {};

var Module = (typeof Module !== 'undefined') ? Module : {
  postRun: []
};

if (typeof Module.postRun === 'function')
  Module.postRun = [Module.postRun];

Module.postRun.push(
  () => {
    console.log("JS initializing");
    RAVELER.WASM_ravel = Module.cwrap('ravel', 'string', ['number','number'], {async: true});
    let init = Module.cwrap('init', null, [], {async: true});
    init().then((result) => {
      RAVELER.WASM_buffer = result;
      console.log("JS ready");
    });
  }
);

function waitForModule() {
  return new Promise(resolve => {
    function check() {
      if (typeof RAVELER.WASM_buffer !== 'undefined')
        resolve();
      else
        setTimeout(check, 100);
    }
    check();
  });
}

async function ravel(pixels) {
  if (pixels.length !== IMG_RES*IMG_RES)
    return { error: `Incorrect image dimensions: ${pixels.length}` };

  // Rearrange pixels into order expected by raveler
  let pixelArray = new Uint8ClampedArray(IMG_RES*IMG_RES);
  for (let i=0; i<IMG_RES; i++) {
    for (let j=0; j<600; j++) {
      pixelArray[IMG_RES*(IMG_RES-1-i)+j] = pixels[IMG_RES*i+j];
    }
  }

  await waitForModule();
  Module.HEAPU8.set(pixelArray, RAVELER.WASM_buffer);
  let designJSON = await RAVELER.WASM_ravel(WEIGHT, FRAME_SIZE);
  return JSON.parse(designJSON);
}

function setStop(stop) {
  let canvas = document.getElementById("stop-overlay");
  let ctx = canvas.getContext('2d');
  let w = canvas.width, h = canvas.height;
  ctx.clearRect(0,0,w,h);

  if (RAVELER.coords) {
    document.getElementById("score-overlay-text").style.display = 'block';
    document.getElementById("stop-indicator").textContent = `Lines: ${stop}`;

    let lengthLabel = document.getElementById("length-indicator");
    let threadLength = getThreadLength(RAVELER.coords, 1/600, stop);
    if (threadLength > 1000)
      lengthLabel.textContent = `Thread length: ${(threadLength/1000).toFixed(2)} km`;
    else if (threadLength > 10)
      lengthLabel.textContent = `Thread length: ${threadLength.toFixed()} m`;
    else
      lengthLabel.textContent = `Thread length: ${threadLength.toFixed(2)} m`;
  }

  let scale;

  scale = (v) => (0.95*v+0.025);
  ctx.beginPath();
  ctx.moveTo(w*scale(0), h*scale(0));
  ctx.lineTo(w*scale(0), h*scale(1));
  ctx.lineTo(w*scale(1), h*scale(1));
  ctx.strokeStyle = '#000';
  ctx.lineWidth = 8;
  ctx.stroke();

  scale = (v) => (0.9*v+0.05);
  let x = w*scale(stop/6000);
  ctx.beginPath();
  ctx.moveTo(x, 0);
  ctx.lineTo(x, canvas.height);
  ctx.strokeStyle = '#00F8';
  ctx.lineWidth = 5;
  ctx.stroke();
}

function saveDesign(design) {
  RAVELER.uid = newUID();
  localStorage.setItem(RAVELER.uid, JSON.stringify(design));

  let query = new URLSearchParams();
  query.set("design", RAVELER.uid);
  let anchor = document.getElementById("assistant-link");
  anchor.href = `assistant.html?${query}`;
  anchor.style.display = "block";
  anchor.style.visibility = "visible";
}

async function updateDesign(pixels) {
  let overlay = document.getElementById("overlay");
  overlay.style.display = "block";
  console.log("Applied overlay");

  setTimeout(() => {
    ravel(pixels).then(design => {
      RAVELER.coords = pins2coords(design.pins, IMG_RES);

      let slider = document.getElementById('stop-slider');
      slider.value = 6000;
      setStop(6000);

      let rCanvas = document.getElementById("raveled");
      drawPath(rCanvas, RAVELER.coords, slider.value, false);

      let sCanvas = document.getElementById("score");
      drawScores(sCanvas, design.scores);

      saveDesign(design);

      overlay.style.display = "none";
    });
  }, 0);
}

function attachFileUploadListener() {
  const ctx = document.getElementById("original").getContext("2d");
  const upload = document.getElementById("upload");

  upload.addEventListener("change", () => {
    let reader = new FileReader();
    reader.addEventListener("load", () => {
      const img = new Image();
      img.addEventListener("load", () => {
        let wh = ctx.canvas.width;
        let iw = img.width, ih = img.height;

        // Size of crop region (in source coordinates)
        let sw = Math.min(iw, ih);

        // Top left corner of visible region (in source coordinates)
        let sx = (iw-sw)/2;
        let sy = (ih-sw)/2;

        ctx.clearRect(0, 0, wh, wh);
        ctx.drawImage(img, sx, sy, sw, sw, 0, 0, wh, wh);

        // Returns pixel data as [r,g,b,a,r,g,b,a,r,...,a]
        let canvasImg = ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height);
        let pixels = rgba2gray(canvasImg.data);
        updateDesign(pixels);
      });
      img.src = reader.result;
    });
    reader.readAsDataURL(upload.files[0]);
  });
}

function attachStopSliderListener() {
  let slider = document.getElementById('stop-slider');
  slider.addEventListener("input", () => {
    setStop(slider.value);

    if (RAVELER.coords) {
      let rCanvas = document.getElementById("raveled");
      drawPath(rCanvas, RAVELER.coords, slider.value, false);

      let query = new URLSearchParams();
      query.set("design", RAVELER.uid);
      query.set("stop", slider.value);

      let anchor = document.getElementById("assistant-link");
      anchor.href = `assistant.html?${query}`;
      anchor.style.display = "block";
      anchor.style.visibility = "visible";
    }
  });
}

window.addEventListener("load", () => {
  attachFileUploadListener();
  attachStopSliderListener();
  setStop(6000);
});