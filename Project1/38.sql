select P.name
from Pokemon as P, (
  select after_id as pid
  from Evolution
  where after_id not in (
    select before_id
    from Evolution
    )
  ) as E
where P.id = E.pid
order by P.name
