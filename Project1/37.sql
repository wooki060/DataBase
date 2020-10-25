select name, C.lv
from Trainer as T, (
  select owner_id, sum(level) as lv
  from CatchedPokemon
  group by owner_id
  having sum(level) >=  (
    select sum(level)
    from CatchedPokemon
    group by owner_id
    order by sum(level) desc
    limit 1
    )
  ) as C
where T.id = C.owner_id
order by name
