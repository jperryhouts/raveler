"use strict";

function rgba2gray(pixels) {
  let gray = new Uint8ClampedArray(pixels.length/4);
  for (var i=0; i < gray.length; i++) {
    gray[i] = parseInt(pixels[i*4]*.299 + pixels[i*4+1]*.587 + pixels[i*4+2]*.114);
  }
  return gray;
}

function pins2coords(pins, scale) {
  return pins.map(pinNumber => {
    let theta = 2.0 * Math.PI * pinNumber / 300;
    let x = scale*(0.5 + Math.sin(theta)/2.0);
    let y = scale*(0.5 - Math.cos(theta)/2.0);
    return [x, y];
  });
}

function newUID() {
  while (true) {
    let uid = Math.random().toString(36).substring(2);
    if (!localStorage.getItem(uid)) {
      return uid;
    } else {
      console.warn('Avoided id collision', uid);
    }
  }
}

function drawScores(canvas, scores) {
  let ctx = canvas.getContext('2d');
  let w = ctx.canvas.width;
  let h = ctx.canvas.height;
  console.log(w, h);

  let xRange = [0, scores.length];
  let yRange = scores.reduce((pVal, cVal) => {
    let yMin = Math.min(pVal[0], cVal);
    let yMax = Math.max(pVal[1], cVal);
    return [yMin, yMax];
  }, [Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY]);

  scores = scores.map(score => {
    return (1 - (score-yRange[0])/(yRange[1]-yRange[0]));
  });

  let scale = (v) => (0.9*v+0.05);

  let path = new Path2D();
  path.moveTo(w*scale(0), h*scale(scores[0]));
  scores.forEach((score, idx) => {
    let x = w*scale(idx/xRange[1]);
    let y = h*scale(score);
    path.lineTo(x, y);
  });

  ctx.clearRect(0, 0, w, h);
  ctx.strokeStyle = '#000';
  ctx.lineWidth = 1;
  ctx.stroke(path);
}

function drawPath(canvas, coords, stop, scale, weight, highlight) {
  let path = new Path2D();
  path.moveTo(...coords[0]);
  let coo = coords.filter((v, i) => (i > 0) && (i < stop || stop === null));
  for (let c of coo)
    path.lineTo(...c);

  let ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, scale, scale);
  ctx.strokeStyle = '#000';
  ctx.lineWidth = scale * weight;
  ctx.stroke(path);

  if (stop && highlight) {
    // Draw the final line in red
    ctx.beginPath();
    ctx.moveTo(...coords[stop-1]);
    ctx.lineTo(...coords[stop]);
    ctx.strokeStyle = '#F00';
    ctx.lineWidth = scale/300;
    ctx.stroke();

    // Draw a circle at the current pin
    ctx.beginPath();
    ctx.moveTo(...coords[stop-1]);
    ctx.arc(...coords[stop], 4, 0, 2*Math.PI, );
    ctx.fillStyle = '#F00';
    ctx.fill();
  }
}