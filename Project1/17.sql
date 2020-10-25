select count(distinct pid)
from CatchedPokemon
where owner_id in (
  select id
  from Trainer
  where hometown = 'Sangnok City'
)
