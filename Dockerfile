FROM jekyll/jekyll

COPY --chown=jekyll:jekyll Gemfile .

RUN bundle install --quiet

CMD ["jekyll", "serve"]
