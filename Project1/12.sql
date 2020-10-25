select name, type
from Pokemon
where id in (
  select pid
  from CatchedPokemon
  where level >= 30)
order by name
