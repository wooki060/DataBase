select sum(level)
from CatchedPokemon
where owner_id = (
  select id
  from Trainer
  where name = 'Matis'
  )
