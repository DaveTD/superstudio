# Superstudio

The Superstudio gem aims to let ruby web developers create sensible JSON in a fast, memory-efficient and developer-friendly way.

## Quickstart
In your `Gemfile`:
```ruby
gem 'superstudio'
```

In your command line:
```sh
$ bundle install
$ rails g superstudio:schema <class name>
$ rails g superstudio:schema_map <JSON schema created from last line>
```

You should now see a `<class name>.json.schema` file (created by the `schema` generator) and a `<class name>Mapper.rb` file that contains a new class (created by the `schema_map` generator) in a new `app/json_schemas` directory.

In a controller:
```ruby
def <method>
  test_query = <ClassName>.all
  test_output = <MapperClassName>.new(test_query.to_sql)
  render json: test_output.json_result
end
```

Add a route to your `routes.rb` file that points to that controller method.

When you hit that route, you should now see JSON output for everything you've stored of that object type.

### Additional Information
- Additional information on using the Superstudio gem can be found [here](https://github.com/DaveTD/superstudio/wiki)
- Information on which parts of JSON schema are supported can be found [here](https://github.com/DaveTD/superstudio/wiki/JSON-Schema-Support).
- Information on how the `Mapper` classes work can be found [here](https://github.com/DaveTD/superstudio/wiki/Mapper-Classes).
- Other known limitations can be found [here](https://github.com/DaveTD/superstudio/wiki/Known-Limitations).


### Why The Name/Why is "Studio" Not Capitalized?
"Superstudio" is named after the Italian architecture firm [Superstudio](https://en.wikipedia.org/wiki/Superstudio). Superstudio's geometric "anti-designs" served both as a commentary about the excesses of pop design, and as a warning about how trends in city planning and construction were lacking humanity and culture.

The Superstudio gem is an anti-design to trends in JSON generation and API design. It rejects common API design choices like cacheing and complicated DSLs. Instead, the gem focuses on creating a minimal and effective solution to the problem of JSON generation.


## LICENSE


Copyright (C) 2017  David Dawson

Superstudio is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Superstudio is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Superstudio.  If not, see <http://www.gnu.org/licenses/>.