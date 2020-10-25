select P.name, P.id
from CatchedPokemon as C, Pokemon as P
where C.owner_id in (
  select id
  from Trainer
  where hometown = 'Sangnok City')
and C.pid = P.id
order by C.pid
