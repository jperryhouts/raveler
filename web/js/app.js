"use strict";

const N_COLUMNS = 6;
const ROWS_PER_BLOCK = 10;

const THREAD_DIA = 150e-6;
const IMG_RES = 500;

function mkBlock(elements, start, nCols, maxRows) {
  let block = document.createElement('div');
  block.className = 'divTable';
  for (let i=0; i<maxRows; i++) {
    let blockRowNumber = i+1;

    let row = document.createElement('div');
    row.className = 'divRow';
    row.classList.add(blockRowNumber%2 === 0 ? 'even' : 'odd');

    let e0 = document.createElement('div');
    e0.classList.add('divCell', 'label');
    e0.textContent = (start/nCols + blockRowNumber).toString();
    row.appendChild(e0);

    for (let j=0; j<nCols; j++) {
      let idx = start + i*nCols + j;
      if (idx < elements.length)
        row.appendChild(elements[idx]);
      else
        break;
    }
    block.appendChild(row);

    if (start+nCols*(i+1) >= elements.length)
      break;
  }

  return block;
}

function getCurrent() {
  let selected = [...document.getElementsByClassName('activeCell')];
  if (selected.length === 1) {
    let current = selected[0];
    let re = /pin-([0-9]*)/.exec(current.id);
    if (re)
      return Number(re[1]);
    else
      console.error('Unable to determine index:', current);
  } else {
    return -1;
  }
}

function drawPath(canvas, coords, stop) {
  let path = new Path2D();
  path.moveTo(...coords[0]);
  let coo = coords.filter((v, i) => (i > 0) && (i < stop || stop === null));
  for (let c of coo)
    path.lineTo(...c);

  let ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, IMG_RES, IMG_RES);
  ctx.strokeStyle = '#000';
  ctx.lineWidth = IMG_RES * THREAD_DIA;
  ctx.stroke(path);

  if (stop) {
    // Draw the final line in red
    ctx.beginPath();
    ctx.moveTo(...coords[stop-1]);
    ctx.lineTo(...coords[stop]);
    ctx.strokeStyle = '#F00';
    ctx.lineWidth = IMG_RES/300;
    ctx.stroke();

    // Draw a circle at the current pin
    ctx.beginPath();
    ctx.moveTo(...coords[stop-1]);
    ctx.arc(...coords[stop], 4, 0, 2*Math.PI, );
    ctx.fillStyle = '#F00';
    ctx.fill();
  }
}

function speak(pinNumber) {
  let clip = document.getElementById(`sound-bite-${pinNumber}`);
  clip.currentTime = 0;
  clip.play();
}

async function init() {
  ///////////////////////////////
  // Pre-load the sound bites
  //////////////////////////////
  for (let n=0; n<300; n++) {
    let nPad = String(n).padStart(3, '0');
    let clip = document.createElement('audio');
    clip.setAttribute('id', `sound-bite-${n}`);
    let types = {
      opus: 'audio/ogg; codecs=opus',
      ogg: 'audio/ogg; codecs=vorbis',
      mp3: 'audio/mpeg',
      wav: 'audio/wav'
    };
    for (let fmt of ['opus', 'ogg', 'mp3', 'wav']) {
      let src = document.createElement('source');
      src.setAttribute('src', `web/sounds_${fmt}/${nPad}.${fmt}`);
      src.setAttribute('type', types[fmt]);
      clip.appendChild(src);
    }
    clip.load();
    clip.volume = 0.65;
    document.body.appendChild(clip);
  }

  const uri = 'web/pattern.json';
  const design = await fetch(uri).then(res => res.json());

  // Provides closure for the coords variable.
  const setCurrent = (idx) => {
    [...document.getElementsByTagName('div')].forEach(e => {
      e.classList.remove('activeCell');
      e.parentNode.classList.remove('activeRow');
    });

    let selected = document.getElementById(`pin-${idx}`);
    selected.classList.add('activeCell');
    selected.parentNode.classList.add('activeRow');
    selected.scrollIntoView({ block: 'center' });

    let currentPin = selected.textContent;
    speak(currentPin);

    let canvas = document.getElementById("partImg");
    drawPath(canvas, coords, idx);
  }

  ////////////////////////////////
  // Now, actually fill the table with this sequence
  // of pin indices.
  ////////////////////////////////
  let sequence = design.pinSequence.map((pinNumber, idx) => {
    let e = document.createElement('div');
    e.setAttribute('id', `pin-${idx}`);
    e.textContent = pinNumber.toString();
    e.classList.add('divCell', 'item');
    e.addEventListener('click', () => { setCurrent(idx); });
    return e;
  });

  // The table is made of a series of independent blocks, which
  // we need to initialize individually.
  let listRootElement = document.getElementById('pinList');
  // Clear out any existing child elements.
  while (listRootElement.firstChild)
    listRootElement.removeChild(listRootElement.firstChild);

  // Now, build up the big table out of a sequence of smaller blocks
  let blockStart=0
  while (blockStart < sequence.length) {
    let block = mkBlock(sequence, blockStart, N_COLUMNS, ROWS_PER_BLOCK);
    listRootElement.appendChild(block);
    blockStart += N_COLUMNS*ROWS_PER_BLOCK;
  }

  ////////////////////////////////
  // Map pin sequence to coordinates and store in global
  // scope..
  ///////////////////////////////
  let coords = design.pinSequence.map(pinNumber => {
    let theta = 2.0 * Math.PI * pinNumber / 300;
    // TODO: This is backwards on purpose for now.
    let x = IMG_RES*(0.5 - Math.sin(theta)/2.0);
    let y = IMG_RES*(0.5 + Math.cos(theta)/2.0);
    return [x, y];
  });

  // Now, draw the full example image. The 'partial' image
  // will be updated via `setCurrent()` as the user moves
  // along with the pin sequence.
  let canvas = document.getElementById("fullImg");
  if (canvas.getContext) {
    drawPath(canvas, coords, null);
  }

  // Restore previous
  let binder = document.getElementById("incrementBinding");
  if (localStorage.getItem("incrementBinding")) {
    binder.value = localStorage.getItem("incrementBinding");
  }
  binder.addEventListener('keyup', () => {
    localStorage.setItem("incrementBinding", binder.value);
  });

  let debounceLock = false;

  // Finally, listen for user input.
  document.addEventListener('keypress', (evt) => {
    let binder = document.getElementById("incrementBinding");
    if (evt.target === binder) {
      return;
    } else if (debounceLock) {
      evt.preventDefault();
      return;
    }

    let code = binder.value.toLowerCase();
    let acceptable = evt.key.toLowerCase() == code ||
                    evt.code.toLowerCase() == code;
    if (code && acceptable) {
      evt.preventDefault();
      let current = getCurrent();
      setCurrent(current+1);
    } else {
      console.log(`Received keypress: ${evt.code} / ${evt.key}`,
                  `Only listening for "${binder.value}". Ignoring.`);
    }

    debounceLock = true;
    window.setTimeout(() => {
      debounceLock = false;
    }, 1000);
  });
}

window.addEventListener('load', init);
