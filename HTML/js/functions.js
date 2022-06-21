/*Add the JavaScript here for the function billingFunction().  
It is responsible for setting and clearing the fields in 
Billing Information */

var northSouthPos = 0; //south is negative values, north is positive values
var eastWestPos = 0; // east is positive, west is negative
var vel = 0;
var actualOrientation = 0;
var actualAltitud = 0;
var labelVelActualHTML = "Velocidad actual: ";
var labelOriActualHTML =  "Azimut actual: ";
var labelAltActualHTML =  "Altitud actual: ";


function setLabels() {
  document.getElementById(labelVelActual).innerHTML = labelVelActualHTML;
  document.getElementById(labelOriActual).innerHTML = labelOriActualHTML;
  document.getElementById(labelAltActual).innerHTML = labelAltActualHTML;
}

function updateLabelsManual() {
  window.alert("got into updateLabels");
  document.getElementById(labelVelActual).innerHTML = labelVelActualHTML + vel;
  document.getElementById(labelOriActual).innerHTML = labelOriActualHTML + actualOrientation;
  document.getElementById(labelAltActual).innerHTML = labelAltActualHTML + actualAltitud;
}



function manualBtnEvent(btn) {
  var btnValue = btn.value;
  switch (btnValue) {
    case 'Norte':
      actualOrientation++;
      break;
    case 'Sur':
      actualOrientation--;
      break;
    case 'Este':
      actualAltitud++;
      break;
    case 'Oeste':
      actualAltitud--;
      break;
    default:
      //
  }
  window.alert(btnValue);
  vel++;
  updateLabelsManual()
}

function simulateDay() {

  if (document.getElementById("simulateDayCheckBox").checked) {
    window.alert("simulateDayCheckBox is checked");
    document.getElementById("current_time").disabled = true;
    document.getElementById("comboBoxLocations").disabled = false;
  }
  else {
    window.alert("simulateDayCheckBox is NOT checked");
    document.getElementById("current_time").disabled = false;
    document.getElementById("comboBoxLocations").disabled = true;
  }

}

