select avg(level)
from CatchedPokemon
where owner_id = (
  select id
  from Trainer
  where name = 'Red')
