enum precipitation {mist, sprinkle, drizzle, rain, shower};
var todaysWeather: precipitation = precipitation.sprinkle;

var f = open("_test_fwritelnEnumFile.txt", mode.w).writer();

f.writeln(todaysWeather);

f.close();
