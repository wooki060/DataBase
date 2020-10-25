select AVG(level)
from Trainer as T, CatchedPokemon as C
where T.id = C.owner_id
and T.hometown = 'Sangnok City'
and C.pid in (
  select id
  from Pokemon as P
  where P.type = 'Electric')
  
