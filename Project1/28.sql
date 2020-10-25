select T.name, avg(C.level)
from Trainer as T, CatchedPokemon as C
where T.id = C.owner_id
and C.pid in (
  select id
  from Pokemon
  where type = 'Normal' or type = 'Electric'
  )
group by T.name
order by avg(C.level)
