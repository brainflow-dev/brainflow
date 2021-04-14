$(document).ready(function() {
  const dropdownManufactorers = $("#manufactorer-dropdown-container ul li")
    .toArray()
    .map((li) => li.innerText);

  function label(lbl) {
    lbl = lbl.replace(/[ .]/g, '-').toLowerCase();
    return lbl;
  }

  function urlSearchParams(searchString) {
    let searchDict = new Map();
    let searchParams = searchString.substring(1).split("&");
    searchParams.forEach(function(element) {
      kvPair = element.split("=");
      if (kvPair[0] === 'manufactorer' &&
          dropdownManufactorers.indexOf(kvPair[1]) == -1) {
        kvPair[1] = dropdownManufactorers[0];
      }
      searchDict.set(kvPair[0], kvPair[1]);
    });
    return searchDict;
  }

  function is_a_match(elem, text) {
    if (label(elem.text()).includes(label(text))) {
      elem.addClass(('active'))
    }
  }

  function setSelects(urlParams, dontPushState) {
    let queryString = '?';
    $('button.opt').removeClass('active');
    if (urlParams.get('manufactorer')) {
      manufactorerSelect = urlParams.get('manufactorer');
      $('li.manufactorers').removeClass('active');
      $('li.manufactorers')
          .each(function() { is_a_match($(this), manufactorerSelect) });
      $('.current-manufactorer')
          .html(
              manufactorerSelect +
              '<svg class="dropdown-caret" viewBox="0 0 32 32" class="icon icon-caret-bottom" aria-hidden="true"><path class="dropdown-caret-path" d="M24 11.305l-7.997 11.39L8 11.305z"></path></svg>');
      queryString += 'manufactorer=' + manufactorerSelect + '&';
    }
    if (urlParams.get('platform')) {
      platformSelect = label(urlParams.get('platform'));
      $('button.opt').each(function() { is_a_match($(this), platformSelect) });
      queryString += 'platform=' + platformSelect + '&';
    }
    if (urlParams.get('language')) {
      languageSelect = label(urlParams.get('language'));
      $('button.opt').each(function() {
        if (label($(this).text()) === label(languageSelect)) {
          $(this).addClass(('active'))
        }
      });
      queryString += 'language=' + languageSelect + '&';
    }
    if (urlParams.get('environ')) {
      environSelect = label(urlParams.get('environ'));
      $('button.opt').each(function() { is_a_match($(this), environSelect) });
      queryString += 'environ=' + environSelect + '&';
    }
    if (urlParams.get('iot')) {
      iotSelect = label(urlParams.get('iot'));
      $('button.opt').each(function() { is_a_match($(this), iotSelect) });
      queryString += 'iot=' + iotSelect + '&';
    }
    if (urlParams.get('board')) {
      boardSelect = label(urlParams.get('board'));
      $('button.opt').each(function() { is_a_match($(this), boardSelect) });
      queryString += 'board=' + boardSelect + '&';
    }

    showContent();

    if (window.location.href.indexOf("/get_started") >= 0 && !dontPushState) {
      history.pushState(null, null, queryString);
    }
  }

  function showContent() {
    $('.opt-group .opt')
        .each(function() { $('.' + label($(this).text())).hide(); });
    $('.opt-group .active')
        .each(function() { $('.' + label($(this).text())).show(); });
  }

  setSelects(urlSearchParams(window.location.search));

  function setContent() {
    var el = $(this);
    let urlParams = urlSearchParams(window.location.search);
    el.siblings().removeClass('active');
    el.addClass('active');
    if ($(this).hasClass("manufactorers")) {
      $('.current-manufactorer').html($(this).text());
      urlParams.set("manufactorer", $(this).text());
    } else if ($(this).hasClass("platforms")) {
      urlParams.set("platform", label($(this).text()));
    } else if ($(this).hasClass("languages")) {
      urlParams.set("language", label($(this).text()));
    } else if ($(this).hasClass("environs")) {
      urlParams.set("environ", label($(this).text()));
    } else if ($(this).hasClass("iots")) {
      console.log($(this));
      urlParams.set("iot", label($(this).text()));
    } else if ($(this).hasClass("boards")) {
      console.log($(this));
      urlParams.set("board", label($(this).text()));
    }
    setSelects(urlParams);
  }

  $('.opt-group').on('click', '.opt', setContent);
  $('.install-widget').css("visibility", "visible");
  $('.install-content').css("visibility", "visible");
  $(window).on('popstate', function() {
    setSelects(urlSearchParams(window.location.search), true);
  });

  let timer;
  const toggleDropdown =
      function(showContent) {
    if (timer)
      clearTimeout(timer);
    if (showContent) {
      timer =
        setTimeout(function() { $(".manufactorer-dropdown").show() }, 250);
    } else {
      $(".manufactorer-dropdown").hide()
    }
  }

  $("#manufactorer-dropdown-container")
    .mouseenter(toggleDropdown.bind(null, true))
    .mouseleave(toggleDropdown.bind(null, false))
    .click(function() { $(".manufactorer-dropdown").toggle() });

  $("ul.manufactorer-dropdown").click(function(e) { e.preventDefault(); });
});
