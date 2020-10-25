select T.name, catch_max.max_level
from Trainer as T, (
  select owner_id, max(level) as max_level
  from CatchedPokemon
  group by owner_id
  having count(owner_id) >= 4
  ) as catch_max
where T.id = catch_max.owner_id
order by T.name
