"use strict";

const browserify = require("browserify");
const buffer = require("vinyl-buffer");
const connect = require("gulp-connect");
const cssModulesify = require("css-modulesify");
const DtsCreator = require("typed-css-modules");
const glob = require("glob");
const gulp = require("gulp");
const gutil = require("gulp-util");
const gzip = require("gulp-gzip");
const karmaServer = require("karma").Server;
const source = require("vinyl-source-stream");
const sourcemaps = require("gulp-sourcemaps");
const tslint = require("gulp-tslint");
const uglify = require("gulp-uglify");

const prod = process.env.ENV === "prod";

const postcss = [ // order matters
  require("postcss-import")({
    path: ["./src/"]
  }),
  require("postcss-cssnext")({
    warnForDuplicates: !prod
  }),
  require("postcss-assets")({
    loadPaths: ["./src/"]
  }),
];

module.exports = {
  postcss
};

gulp.task("bundle", ["create-dirs", "style-type-definitions"], (done) => {
  const bundle = browserify("./src/app/main.tsx", { debug: !prod })
    .plugin(require("tsify"))
    .plugin(cssModulesify, {
      before: prod ? postcss.concat(require("cssnano")) : postcss,
      global: true,
      output: "./dist/static/main.css",
      rootDir: __dirname,
      generateScopedName: prod ? cssModulesify.generateShortName : cssModulesify.generateLongName,
    })
    .bundle()
    .on("error", gutil.log)
    .pipe(source("main.js"))
    .pipe(buffer());
  if (prod) {
    bundle
      .pipe(uglify())
      .on("error", gutil.log);
  } else {
    bundle
      .pipe(sourcemaps.init({loadMaps: true}))
      .pipe(sourcemaps.write());
  }
  bundle
    .pipe(gulp.dest("./dist/static/"))
    .on("end", done);
});

gulp.task("lint", () => {
  gulp
    .src(["./src/**/*.ts", "./src/**/*.tsx"])
    .on("error", gutil.log)
    .pipe(tslint({
      fix: true,
      formatter: "verbose",
    }))
    .pipe(tslint.report());
});

gulp.task("create-dirs", () => {
  gulp.src("")
    .pipe(gulp.dest("./dist/static/"));
});

gulp.task("style-type-definitions", (done) => {
  let creator = new DtsCreator({
    searchDir: "./src"
  });
  glob("./src/**/*.css", null, (err, files) => {
    if (err) {
      console.error(err);
      return;
    }
    if (!files || !files.length) return;
    Promise
      .all(files.map((f) => {
        return creator
          .create(f)
          .then((content) => content.writeFile())
          .then((content) => {
            console.log("Wrote " + gutil.colors.green(content.outputFilePath));
            content.messageList.forEach((message) => {
              console.warn(gutil.colors.yellow("[Warn] " + message));
            });
          })
          .catch((reason) => console.error(gutil.colors.red("[Error] " + reason)));
      }))
      .then(() => done());
  });
});

gulp.task("html", () => {
  gulp
    .src(["./src/index.html"])
    .on("error", gutil.log)
    .pipe(gulp.dest("./dist/"));
});

gulp.task("test", ["style-type-definitions", "lint"], (done) => {
  new karmaServer({
    configFile: __dirname + "/karma.conf.js",
    singleRun: true,
  }, done).start();
});

gulp.task("test-ci", ["style-type-definitions", "lint"], (done) => {
  new karmaServer({
    configFile: __dirname + "/karma.conf.js",
    singleRun: true,
    browsers: ["HeadlessChrome"],
  }, done).start();
});

gulp.task("test-watch", ["style-type-definitions", "lint"], (done) => {
  new karmaServer({
    configFile: __dirname + "/karma.conf.js",
  }, done).start();
});

gulp.task("develop", ["html", "bundle"]);

gulp.task("reload", ["develop"], () => {
  gulp
    .src("./dist/")
    .pipe(connect.reload());
});

gulp.task("watch", ["develop"], () => {
  gulp.watch("./src/**/*.*", { debounceDelay: 2000 }, ["reload"]);
  connect.server({
    host: "0.0.0.0",
    livereload: true,
    port: 8888,
    root: "./dist/",
  });
});

gulp.task("release", ["develop"], () => {
  gulp
    .src(["./dist/**/*.html", "./dist/**/*.js", "./dist/**/*.css"])
    .pipe(gzip())
    .pipe(gulp.dest("./dist/"));
});

gulp.task("default", ["develop"]);
