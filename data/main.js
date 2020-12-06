setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("time").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/time", true);
    xhttp.send();
  }, 10000 ) ;

  function setHodiny() {
    var hh = $('#hodinyTest').val();
      $.post("/hodiny", { hodiny: hh });
  }

  function setMinuty() {
    var mm = $('#minutyTest').val();
      $.post("/minuty", { minuty: mm });
  }

  $(document).ready(() => {
    $("#range").on("input", () => {
      $("#currentAngle").text("Current Value : " + $("#range").val() + "°");
      $.post("/angle", {
          angle: $("#range").val(),
      });
    });
  });