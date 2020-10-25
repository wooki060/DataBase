select name
from Trainer
where id in (
  select owner_id
  from CatchedPokemon
  group by owner_id, pid
  having count(pid) >= 2
  )
order by name
