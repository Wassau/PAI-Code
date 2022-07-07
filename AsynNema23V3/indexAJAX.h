/*


  OK, ya ready for some fun? HTML + CSS styling + javascript all in and undebuggable environment

  one trick I've learned to how to debug HTML and CSS code.

  get all your HTML code (from html to /html) and past it into this test site
  muck with the HTML and CSS code until it's what you want
  https://www.w3schools.com/html/tryit.asp?filename=tryhtml_intro

  No clue how to debug javascrip other that write, compile, upload, refresh, guess, repeat

  I'm using class designators to set styles and id's for data updating
  for example:
  the CSS class .tabledata defines with the cell will look like
  <td><div class="tabledata" id = "switch"></div></td>

  the XML code will update the data where id = "switch"
  java script then uses getElementById
  document.getElementById("switch").innerHTML="Switch is OFF";


  .. now you can have the class define the look AND the class update the content, but you will then need
  a class for every data field that must be updated, here's what that will look like
  <td><div class="switch"></div></td>

  the XML code will update the data where class = "switch"
  java script then uses getElementsByClassName
  document.getElementsByClassName("switch")[0].style.color=text_color;


  the main general sections of a web page are the following and used here

  <html>
    <style>
    // dump CSS style stuff in here
    </style>
    <body>
      <header>
      // put header code for cute banners here
      </header>
      <main>
      // the buld of your web page contents
      </main>
      <footer>
      // put cute footer (c) 2021 xyz inc type thing
      </footer>
    </body>
    <script>
    // you java code between these tags
    </script>
  </html>


*/

// note R"KEYWORD( html page code )KEYWORD";
// again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en" class="js-focus-visible">

<head>
    <meta charset="UTF-8">
    <title>Heliodon</title>

    <style>
        header {
            text-align: center;
            color: #FFFFFF;
            background-color: #008000;
        }

        

        label {
            display: inline;
            text-align: right;
        }

        form {
            display: inline-block;
            width: 30%;
            padding: 5px;
            -webkit-column-count: 1;
            -moz-column-count: 1;
            -o-column-count: 1;
            column-count: 1;

            -webkit-column-gap: 15px;
            -moz-column-gap: 15px;
            -o-column-gap: 15px;
            column-gap: 15px;

            -webkit-border-radius: 3;
            -moz-border-radius: 3;
            -o-border-radius: 3;
            border-radius: 25px;

            background: #00b7ea;
            /* Old browsers */
            background: -moz-linear-gradient(top, #00b7ea 0%, #D2E4E8 100%);
            background: -webkit-linear-gradient(#00b7ea 0%, #D2E4E8 100%);
            background: -o-linear-gradient(#00b7ea 0%, #D2E4E8 100%);
            background: linear-gradient(#00b7ea 0%, #D2E4E8 100%);
        }

        div {
            margin-bottom: 10px;
        }

        body {
            text-align: center;
            background-color: #B3B3B3;
        }

        footer {
            text-align: center;
            color: #FFFFFF;
            background-color: #008000;

        }

        p {
            color: red;
            padding: 10px 5px;
            background: black;
        }

        .results {
            display: inline-block;
            color: "FFFFFF";
            background-color: #808080;
        }



         .notDisplayOnLoad{
            display: none;
         }

          
        .button {
            background-color: #1c87c9;
            border-width: 5px;
            color: white;
            padding: 20px 34px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 20px;
            margin: 5px 10px;
            cursor: pointer;
        }
        
        .fancy {
            font-family: cursive;
            background: red;
            color: green;
        }

        .plain {
            font-family: Times, serif;
            color: black;
        }
    </style>

</head>

<body onload="process()">
    <header>
        <h1>Proyecto Helidon</h1>
    </header>
    
    <form id="formManual" class="notDisplayOnLoad">
        <div>
            <input type="radio" id="move_manual" name="type_move" value="manual" checked="checked" onclick="manualDisplay()">
            <label for="move_manual">Manual</label>
            <input type="radio" id="move_auto" name="type_move" value="automatico" onclick="automaticDisplay()">
            <label for="move_auto">Automatico</label><br><br>
        </div>
        <div>
            <input type="button" onclick="ButtonPressNORTH(this)" id="33" value="Norte">
        </div>
        <div>            
            <input type="button" onclick="ButtonPressWEST(this)" id="32" value="Oeste">
            <input type="button" onclick="ButtonPressEAST(this)" id="26" value="Este">
        </div>
        <div>
            <input type="button" onclick="ButtonPressSOUTH(this)" id="25" value="Sur">
        </div>
    </form>

    <form id="formAutomatic" action="/get">
        <div>
            <input type="radio" class="input" id="IsMoveManual" name="type_move" value="manual"
                onclick="manualDisplay()">
            <label for="IsMoveManual">Manual</label>
            <input type="radio" id="IsMoveAuto" name="type_move" value="automatico" onclick="automaticDisplay()">
            <label for="IsMoveAuto">Automatico</label><br><br>
        </div>
        <div>
            <label for="simulateDayCheckBox"><input type="checkbox" id="simulateDayCheckBox" value="first_checkbox"
                    onclick="simulateDay()"> Simular dia</label><br>
        </div>
        <div>
            <label for="lat">Latitud:</label>
            <input type="number" min="-80" max="80" step='0.001' id="lat" class="input" name="lat" value="4.637"
                required><br>
        </div>
        <div>
            <label for="lng">Longitud:</label>
            <input type="number" min="-170" max="170" step='0.001' id="lng" class="input" name="lng" value="-74.083"
                required>
            <br><br>
        </div>

        <label for="current_day">Fecha:</label>
        <input type="date" min="2000-01-01" max="2030-12-31" class="input" id="current_day" name="current_day"
            required><br><br>
        </div>
        <div>
            <label for="current_time">Tiempo:</label>
            <input type="time" min="06:10" max="17:50" class="input" id="current_time" name="current_time"
                required><br><br>
        </div>
        <div>
            <label for="comboBoxLocations" disabled="disabled">Elegir zona horaria:</label>
            <select name="comboBoxLocations" id="comboBoxLocations" disabled="disabled">
                <option value="Bogotá">Bogotá (4.637,-74.083)</option>
                <option value="Ecuador">Ecuador (0,0)</option>
                <option value="Capricornio">Trópico de Capricornio (-23.438,0)</option>
                <option value="Cancer">Trópico de Cancer (23.437,0)</option>
            </select>
        </div>
        <div>
            <input type="submit" value="Enter" onclick="submitPrevent(this)">
        </div>
    </form>
</body>

<script type = "text/javascript">

  
    var northSouthPos = 0; //south is negative values, north is positive values
    var eastWestPos = 0; // east is positive, west is negative
    var vel = 0;
    var actualOrientation = 0;
    var actualAltitud = 0;
    var labelVelActualHTML = "Velocidad actual: ";
    var labelOriActualHTML = "Azimut actual: ";
    var labelAltActualHTML = "Altitud actual: ";


    function submitPrevent(event){
      event.preventDefault();
    }
    
    function manualDisplay() {
        document.getElementById("move_manual").checked = true;
        document.getElementById("move_auto").checked = false;
        document.getElementById("formManual").style.display = "inline-block";
        document.getElementById("formAutomatic").style.display = "none";  
    }

    function automaticDisplay() {
        document.getElementById("IsMoveManual").checked = false;
        document.getElementById("IsMoveAuto").checked = true;
        document.getElementById("formManual").style.display = "none";
        document.getElementById("formAutomatic").style.display = "inline-block";
    }

    function setLabels() {
        document.getElementById("labelVelActual").innerHTML = labelVelActualHTML;
        document.getElementById("labelOriActual").innerHTML = labelOriActualHTML;
        document.getElementById("labelAltActual").innerHTML = labelAltActualHTML;
    }

    function updateLabelsManual() {
        document.getElementById("labelVelActual").innerHTML = labelVelActualHTML + vel;
        document.getElementById("labelOriActual").innerHTML = labelOriActualHTML + actualOrientation;
        document.getElementById("labelAltActual").innerHTML = labelAltActualHTML + actualAltitud; 
    }

    function ButtonPressNORTH(element) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/update?output="+element.id+"&state=1", true);
      xhttp.send();
    }

    function ButtonPressSOUTH(element) {
      var xhttp = new XMLHttpRequest();
       xhttp.open("GET", "/update?output="+element.id+"&state=1", true);
      xhttp.send();
    }
    
    function ButtonPressWEST(element) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/update?output="+element.id+"&state=1", true);
      xhttp.send();
    }

    function ButtonPressEAST(element) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/update?output="+element.id+"&state=1", true);
      xhttp.send();
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

    // function to handle the response from the ESP
   

    function process(){
     if(document.getElementById("move_manual").checked == true){
        manualDisplay();
     }
         
      
        // you may have to play with this value, big pages need more porcessing time, and hence
        // a longer timeout
        setTimeout("process()",100);
    }
</script>

</html>



)=====";
