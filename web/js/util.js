/*
  Raveler
  Copyright (C) 2021 Jonathan Perry-Houts

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

"use strict";

const FRAME_SIZE = 0.622;
const WEIGHT = 44.784e-6;
const IMG_RES = 600;

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

function getThreadLength(coords, scale, stop) {
  let factor = FRAME_SIZE;
  if (scale)
    factor *= scale;

  return coords.reduce((total, val, idx, arr) => {
    if (idx == 0) {
      return 0;
    } else if (stop && idx > stop) {
      return total;
    } else {
      let val0 = arr[idx-1];
      let dx = val[0] - val0[0];
      let dy = val[1] - val0[1];
      return total + factor * Math.sqrt(dx*dx + dy*dy);
    }
  }, 0.0);
}

function drawScores(canvas, scores) {
  let ctx = canvas.getContext('2d');
  let w = ctx.canvas.width;
  let h = ctx.canvas.height;

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

function drawPath(canvas, coords, stop, highlight) {
  let path = new Path2D();
  path.moveTo(...coords[0]);
  let coo = coords.filter((v, i) => (i > 0) && (i < stop || stop === null));
  for (let c of coo)
    path.lineTo(...c);

  let ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, IMG_RES, IMG_RES);
  ctx.strokeStyle = '#000';
  // Scaling by about 2.5 seems to approximate
  // how the result will actually turn out.
  ctx.lineWidth = 2.5 * IMG_RES * WEIGHT / FRAME_SIZE;
  ctx.stroke(path);

  if (stop) {
    // Draw the final line in red
    ctx.beginPath();
    ctx.moveTo(...coords[stop-1]);
    ctx.lineTo(...coords[stop]);
    if (highlight) {
      ctx.strokeStyle = '#F00';
      ctx.lineWidth = 2.5;
    }
    ctx.stroke();

    if (highlight) {
      // Draw a circle at the current pin
      ctx.beginPath();
      ctx.moveTo(...coords[stop-1]);
      ctx.arc(...coords[stop], 4, 0, 2*Math.PI, );
      ctx.fillStyle = '#F00';
      ctx.fill();
    }
  }
}